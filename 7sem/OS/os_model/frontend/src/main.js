// ==========================================================================
//  Модель ОС — фронтенд
//  Приём снимков состояния от Go-ядра (kernel.Snapshot) и обновление
//  панели управления: таблица процессов, процессор, память, ввод-вывод.
//
//  Ядро присылает снимки с частотой от 1 мс (1000 такт/с) до 10 с
//  (0.1 такт/с). Отрисовка троттлится: не чаще RENDER_INTERVAL_MS, а при
//  редких событиях показывается сразу, без задержки.
// ==========================================================================

const SLOT_COUNT = 16; // Размер массива PSW (scheduler.DefaultSlots)

// Минимальный интервал между перерисовками (~20 кадров в секунду).
// При высокой тактовой частоте события приходят каждую 1 мс — без
// троттлинга мост WebView и DOM не успевают обрабатывать поток снимков,
// и интерфейс начинает лагать.
const RENDER_INTERVAL_MS = 50;

// --- Ссылки на элементы DOM -------------------------------------------------

const el = {
    pc: document.getElementById('pc-val'),
    speed: document.getElementById('speed-val'),
    currentProc: document.getElementById('current-proc-val'),
    cpuState: document.getElementById('cpu-state-val'),
    command: document.getElementById('command-val'),
    io: document.getElementById('io-val'),
    ioHint: document.getElementById('io-hint'),
    completed: document.getElementById('completed-val'),
    generated: document.getElementById('generated-val'),
    resident: document.getElementById('resident-val'),
    formula: document.getElementById('scheduler-formula'),

    memPercent: document.getElementById('mem-percent'),
    memBar: document.getElementById('mem-bar'),
    memUsed: document.getElementById('mem-used'),
    memTotal: document.getElementById('mem-total'),

    memPercentLg: document.getElementById('mem-percent-lg'),
    memUsedLg: document.getElementById('mem-used-lg'),
    memTotalLg: document.getElementById('mem-total-lg'),
    memFree: document.getElementById('mem-free'),
    memFragments: document.getElementById('mem-fragments'),
    memMap: document.getElementById('mem-map'),

    slotsUsed: document.getElementById('slots-used'),
    residentCount: document.getElementById('resident-count'),
    countRunning: document.getElementById('count-running'),
    countReady: document.getElementById('count-ready'),
    countIo: document.getElementById('count-io'),
    countAbsent: document.getElementById('count-absent'),
    countOther: document.getElementById('count-other'),

    processList: document.getElementById('process-list'),
};

const buttons = {
    increase: document.getElementById('btn-increase'),
    decrease: document.getElementById('btn-decrease'),
    quit: document.getElementById('btn-quit'),
};

// --- Состояния процесса (синхронизированы с Go-моделью) ---------------------

const STATE = {
    ABSENT: 'Отсутствует',
    READY: 'Готов',
    LOADING: 'Загружается',
    ACTIVE: 'Активен',
    INIT_IO: 'Инициализация ввода вывода',
    END_IO: 'Конец ввода (вывода)',
    BLOCK_MEM: 'Блокирован по обращению к памяти',
    BLOCK_IO: 'Блокирован по обращению ко вводу (выводу)',
    SUSPENDED: 'Приостановлен',
};

const CPU_STATE = {
    WORK: 'Работа',
    WAIT: 'Ожидание',
};

const BADGE_CLASS = {
    [STATE.ACTIVE]: 'badge--running',
    [STATE.READY]: 'badge--ready',
    [STATE.LOADING]: 'badge--loading',
    [STATE.INIT_IO]: 'badge--io',
    [STATE.END_IO]: 'badge--io',
    [STATE.BLOCK_MEM]: 'badge--blocked',
    [STATE.BLOCK_IO]: 'badge--blocked',
    [STATE.SUSPENDED]: 'badge--suspended',
    [STATE.ABSENT]: 'badge--absent',
};

// --- Пул строк таблицы (без полной перерисовки на каждом такте) -------------

const rows = [];

function createRow() {
    const tr = document.createElement('tr');

    // Слот, ID, размер, PC, выполнено команд, приоритет.
    const cells = [];
    for (let i = 0; i < 6; i++) {
        const td = document.createElement('td');
        td.className = i === 3 || i === 4 ? 'font-mono' : '';
        tr.appendChild(td);
        cells.push(td);
    }

    const stateCell = document.createElement('td');
    const badge = document.createElement('span');
    badge.className = 'badge';
    stateCell.appendChild(badge);
    tr.appendChild(stateCell);

    el.processList.appendChild(tr);

    return { tr, cells, badge, sig: '' };
}

for (let i = 0; i < SLOT_COUNT; i++) {
    rows.push(createRow());
}

// --- Планировщик отрисовки (троттлинг) --------------------------------------
// Событие update_stats приходит сотни/тысячи раз в секунду, поэтому колбэк
// только запоминает последний снимок, а render() вызывается не чаще
// RENDER_INTERVAL_MS. Если события редкие — рисуем сразу.

let latest = null;
let renderTimer = 0;
let lastRenderAt = -Infinity;
let memMapSignature = '';

function onStats(snapshot) {
    latest = snapshot;
    scheduleRender();
}

function scheduleRender() {
    if (renderTimer !== 0) {
        return; // кадр уже запланирован
    }

    const elapsed = performance.now() - lastRenderAt;
    if (elapsed >= RENDER_INTERVAL_MS) {
        render(); // редкие события — обновляем немедленно
        return;
    }

    renderTimer = setTimeout(() => {
        renderTimer = 0;
        render();
    }, RENDER_INTERVAL_MS - elapsed);
}

// --- Точечное обновление DOM (без лишних записей и перекомпоновок) ----------

function setText(node, value) {
    const text = String(value);
    if (node.textContent !== text) {
        node.textContent = text;
    }
}

function setClass(node, className) {
    if (node.className !== className) {
        node.className = className;
    }
}

function setWidth(node, width) {
    if (node.style.width !== width) {
        node.style.width = width;
    }
}

function formatSpeed(speed) {
    return speed >= 100 ? String(Math.round(speed)) : Number(speed).toFixed(2);
}

function render() {
    lastRenderAt = performance.now();
    if (!latest) return;

    const snap = latest;
    const procs = snap.processes || [];
    const memory = snap.memory || { total: 0, used: 0, free: 0, fragments: 0 };
    const io = snap.io || {};

    // --- Сводные показатели ЦП ---
    setText(el.pc, snap.pc);
    setText(el.speed, formatSpeed(snap.speed));

    // --- Состояние ЦПр ---
    const isWorking = snap.cpuState === CPU_STATE.WORK;
    setText(el.cpuState, snap.cpuState);
    setClass(el.cpuState, isWorking ? 'stat-value text-success' : 'stat-value text-warn');

    // --- Тип выполняемой команды (подпрограмма индикации, ЛР4) ---
    setText(el.command, snap.commandText || 'Нет команды');

    // --- Устройство ввода-вывода ---
    if (io.busy) {
        setText(el.io, `Занято · P${io.processId}`);
        setClass(el.io, 'stat-value stat-value--sm text-warn');
        setText(el.ioHint, `осталось ${io.ticksLeft} из ${io.totalTicks} такт.`);
    } else {
        setText(el.io, 'Свободно');
        setClass(el.io, 'stat-value stat-value--sm text-success');
        setText(el.ioHint, 'Устройство готово');
    }

    // --- Учёт заданий ---
    setText(el.completed, snap.completedTasks ?? 0);
    setText(el.generated, snap.taskCounter ?? 0);
    setText(el.resident, snap.residentTasks ?? 0);

    // --- Формула планировщика ---
    if (snap.formula) {
        setText(el.formula, snap.formula);
        if (el.formula.title !== snap.formula) {
            el.formula.title = snap.formula;
        }
    }

    // --- Таблица процессов ---
    let usedSlots = 0;
    let active = 0;
    let ready = 0;
    let ioCount = 0;
    let absent = 0;
    let other = 0;

    for (let i = 0; i < SLOT_COUNT; i++) {
        const proc = procs[i];
        const row = rows[i];

        if (!proc || proc.state === STATE.ABSENT) {
            absent++;

            // Строку пустого слота перерисовываем только при изменении.
            if (row.sig !== 'absent') {
                row.sig = 'absent';
                row.tr.className = 'row--absent';
                row.cells[0].textContent = `[${i}]`;
                row.cells[1].textContent = '—';
                row.cells[2].textContent = '—';
                row.cells[3].textContent = '—';
                row.cells[4].textContent = '—';
                row.cells[5].textContent = '—';
                setBadge(row.badge, STATE.ABSENT);
            }
            continue;
        }

        usedSlots++;

        if (proc.state === STATE.ACTIVE) {
            active++;
        } else if (proc.state === STATE.READY) {
            ready++;
        } else if (proc.state === STATE.BLOCK_IO || proc.state === STATE.INIT_IO || proc.state === STATE.END_IO) {
            ioCount++;
        } else {
            other++;
        }

        // Сигнатура строки: если ничего не изменилось (в том числе подсветка
        // активного слота), DOM не трогаем вовсе.
        const sig = [
            i === snap.activeIndex ? 1 : 0,
            proc.state,
            proc.id,
            proc.size,
            proc.pc,
            proc.totalCommands,
            proc.prior,
        ].join('|');

        if (row.sig !== sig) {
            row.sig = sig;
            row.tr.className = i === snap.activeIndex ? 'row--running' : '';
            row.cells[0].textContent = `[${i}]`;
            row.cells[1].textContent = proc.id;
            row.cells[2].textContent = proc.size;
            row.cells[3].textContent = proc.pc;
            row.cells[4].textContent = `${proc.pc} / ${proc.totalCommands}`;
            row.cells[5].textContent = proc.prior;
            setBadge(row.badge, proc.state);
        }
    }

    // --- Активный процесс (номер слота + ID) ---
    if (snap.activeIndex >= 0 && procs[snap.activeIndex]) {
        const activeProc = procs[snap.activeIndex];
        setText(el.currentProc, `[${snap.activeIndex}] · ID ${activeProc.id}`);
    } else {
        setText(el.currentProc, '—');
    }

    // --- Счётчики состояний ---
    setText(el.slotsUsed, usedSlots);
    setText(el.residentCount, snap.residentTasks ?? usedSlots);
    setText(el.countRunning, active);
    setText(el.countReady, ready);
    setText(el.countIo, ioCount);
    setText(el.countAbsent, absent);
    setText(el.countOther, other);

    // --- Память ---
    const percent = memory.total > 0 ? Math.min((memory.used / memory.total) * 100, 100) : 0;
    const percentLabel = `${percent.toFixed(1)}%`;

    setText(el.memUsed, memory.used);
    setText(el.memTotal, memory.total);
    setText(el.memPercent, percentLabel);
    setWidth(el.memBar, percentLabel);

    setText(el.memUsedLg, memory.used);
    setText(el.memTotalLg, memory.total);
    setText(el.memFree, memory.free);
    setText(el.memFragments, memory.fragments ?? 0);
    setText(el.memPercentLg, percentLabel);

    renderMemoryMap(memory);
}

// --- Карта оперативной памяти (занятые и свободные блоки) -------------------
// Пересобираем карту только при изменении набора блоков: в остальных тактах
// (например, при выполнении вычислительных команд) она не меняется.

function renderMemoryMap(memory) {
    const used = memory.usedBlocks || [];
    const free = memory.freeBlocks || [];

    const parts = [];
    for (const b of used) {
        parts.push(`u${b.processId}:${b.start}+${b.size}`);
    }
    for (const b of free) {
        parts.push(`f${b.start}+${b.size}`);
    }

    const signature = parts.join(',');
    if (signature === memMapSignature) {
        return;
    }
    memMapSignature = signature;

    const total = Math.max(memory.total || 0, 1);
    const blocks = [];

    for (const b of used) {
        blocks.push({ start: b.start, size: b.size, used: true, pid: b.processId });
    }
    for (const b of free) {
        blocks.push({ start: b.start, size: b.size, used: false });
    }
    blocks.sort((a, b) => a.start - b.start);

    const fragment = document.createDocumentFragment();

    for (const b of blocks) {
        const seg = document.createElement('div');
        seg.className = b.used ? 'mem-seg mem-seg--used' : 'mem-seg mem-seg--free';
        seg.style.width = `${(b.size / total) * 100}%`;
        seg.title = b.used
            ? `P${b.pid}: [${b.start}; ${b.start + b.size}) — ${b.size} ед.`
            : `Свободно: [${b.start}; ${b.start + b.size}) — ${b.size} ед.`;
        fragment.appendChild(seg);
    }

    el.memMap.replaceChildren(fragment);
}

function setBadge(badge, state) {
    const className = `badge ${BADGE_CLASS[state] || ''}`.trim();
    if (badge.className !== className || badge.textContent !== state) {
        badge.className = className;
        badge.textContent = state;
    }
}

// --- Мост к Go-ядру ---------------------------------------------------------

if (window.runtime && window.runtime.EventsOn) {
    window.runtime.EventsOn('update_stats', onStats);
}

// Wails-биндинги: адаптер App живёт в пакете cmd (см. cmd/main.go).
function appBinding() {
    return window.go?.cmd?.App || window.go?.main?.App || null;
}

const increaseSpeed = () => appBinding()?.IncreaseSpeed();
const decreaseSpeed = () => appBinding()?.DecreaseSpeed();
const quitApp = () => appBinding()?.QuitApp();

buttons.increase.addEventListener('click', increaseSpeed);
buttons.decrease.addEventListener('click', decreaseSpeed);
buttons.quit.addEventListener('click', quitApp);

// --- Горячие клавиши (аппаратные прерывания) --------------------------------

window.addEventListener('keydown', (e) => {
    if (e.key === '=' || e.key === '+') {
        increaseSpeed();
        flash(buttons.increase);
    } else if (e.key === '-') {
        decreaseSpeed();
        flash(buttons.decrease);
    } else if (e.key === 'Escape') {
        quitApp();
        flash(buttons.quit);
    }
});

// Визуальный отклик на нажатие с клавиатуры
function flash(btn) {
    if (!btn) return;
    btn.style.transform = 'scale(0.96)';
    setTimeout(() => (btn.style.transform = ''), 130);
}
