// ==========================================================================
//  Модель ОС — фронтенд
//  Приём событий от Go-ядра и обновление панели управления
// ==========================================================================

const TOTAL_MEMORY = 1000; // Базовое значение из Go-модели
const SLOT_COUNT = 16;     // Размер массива PSW

// --- Ссылки на элементы DOM -------------------------------------------------

const el = {
    pc: document.getElementById('pc-val'),
    speed: document.getElementById('speed-val'),
    currentProc: document.getElementById('current-proc-val'),

    memPercent: document.getElementById('mem-percent'),
    memBar: document.getElementById('mem-bar'),
    memUsed: document.getElementById('mem-used'),
    memTotal: document.getElementById('mem-total'),

    memPercentLg: document.getElementById('mem-percent-lg'),
    memBarLg: document.getElementById('mem-bar-lg'),
    memUsedLg: document.getElementById('mem-used-lg'),
    memTotalLg: document.getElementById('mem-total-lg'),
    memFree: document.getElementById('mem-free'),

    slotsUsed: document.getElementById('slots-used'),
    countRunning: document.getElementById('count-running'),
    countReady: document.getElementById('count-ready'),
    countAbsent: document.getElementById('count-absent'),

    processList: document.getElementById('process-list'),
};

const buttons = {
    increase: document.getElementById('btn-increase'),
    decrease: document.getElementById('btn-decrease'),
    quit: document.getElementById('btn-quit'),
};

// --- Состояния процесса -----------------------------------------------------

const STATE = {
    ABSENT: 'Отсутствует',
    READY: 'Готов',
    RUNNING: 'Выполняется',
};

const STATE_CLASS = {
    [STATE.RUNNING]: 'badge--running',
    [STATE.READY]: 'badge--ready',
    [STATE.ABSENT]: 'badge--absent',
};

const ROW_CLASS = {
    [STATE.RUNNING]: 'row--running',
    [STATE.ABSENT]: 'row--absent',
};

// --- Пул строк таблицы (без полной перерисовки на каждом такте) -------------

const rows = [];

function createRow() {
    const tr = document.createElement('tr');

    const cells = [];
    for (let i = 0; i < 5; i++) {
        const td = document.createElement('td');
        td.className = i === 3 ? 'font-mono' : '';
        tr.appendChild(td);
        cells.push(td);
    }

    const stateCell = document.createElement('td');
    const badge = document.createElement('span');
    badge.className = 'badge';
    stateCell.appendChild(badge);
    tr.appendChild(stateCell);

    el.processList.appendChild(tr);

    return { tr, cells, badge };
}

for (let i = 0; i < SLOT_COUNT; i++) {
    rows.push(createRow());
}

// --- Планировщик отрисовки --------------------------------------------------
// При высокой тактовой частоте событий много — рендерим не чаще одного кадра.

let latest = null;
let frameRequested = false;

function onStats(pc, speed, procs) {
    latest = { pc, speed, procs };
    if (!frameRequested) {
        frameRequested = true;
        requestAnimationFrame(render);
    }
}

function formatSpeed(speed) {
    return speed >= 100 ? String(Math.round(speed)) : speed.toFixed(2);
}

function render() {
    frameRequested = false;
    if (!latest) return;

    const { pc, speed, procs } = latest;

    // --- Сводные показатели ЦП ---
    el.pc.textContent = pc;
    el.speed.textContent = formatSpeed(speed);

    let usedMemory = 0;
    let runningProc = null;
    let slotsUsed = 0;
    let running = 0;
    let ready = 0;
    let absent = 0;

    // --- Таблица процессов ---
    for (let i = 0; i < SLOT_COUNT; i++) {
        const proc = procs[i];
        const row = rows[i];

        if (!proc || proc.state === STATE.ABSENT) {
            absent++;
            row.tr.className = ROW_CLASS[STATE.ABSENT] || '';
            row.cells[0].textContent = `[${i}]`;
            row.cells[1].textContent = '—';
            row.cells[2].textContent = '—';
            row.cells[3].textContent = '—';
            row.cells[4].textContent = '—';
            setBadge(row.badge, STATE.ABSENT);
            continue;
        }

        slotsUsed++;
        usedMemory += proc.size;

        if (proc.state === STATE.RUNNING) {
            running++;
            runningProc = proc;
        } else if (proc.state === STATE.READY) {
            ready++;
        }

        row.tr.className = ROW_CLASS[proc.state] || '';
        row.cells[0].textContent = `[${i}]`;
        row.cells[1].textContent = proc.id;
        row.cells[2].textContent = proc.size;
        row.cells[3].textContent = proc.pc;
        row.cells[4].textContent = proc.prior;
        setBadge(row.badge, proc.state);
    }

    // --- Активный процесс ---
    el.currentProc.textContent = runningProc ? `ID ${runningProc.id}` : '—';

    // --- Счётчики состояний ---
    el.slotsUsed.textContent = slotsUsed;
    el.countRunning.textContent = running;
    el.countReady.textContent = ready;
    el.countAbsent.textContent = absent;

    // --- Память ---
    const freeMemory = Math.max(TOTAL_MEMORY - usedMemory, 0);
    const percent = Math.min((usedMemory / TOTAL_MEMORY) * 100, 100).toFixed(1);
    const percentLabel = `${percent}%`;

    el.memUsed.textContent = usedMemory;
    el.memTotal.textContent = TOTAL_MEMORY;
    el.memPercent.textContent = percentLabel;
    el.memBar.style.width = percentLabel;

    el.memUsedLg.textContent = usedMemory;
    el.memTotalLg.textContent = TOTAL_MEMORY;
    el.memFree.textContent = freeMemory;
    el.memPercentLg.textContent = percentLabel;
    el.memBarLg.style.width = percentLabel;
}

function setBadge(badge, state) {
    badge.className = `badge ${STATE_CLASS[state] || ''}`.trim();
    badge.textContent = state;
}

// --- Мост к Go-ядру ---------------------------------------------------------

if (window.runtime && window.runtime.EventsOn) {
    window.runtime.EventsOn('update_stats', onStats);
}

const increaseSpeed = () => window.go?.main?.App?.IncreaseSpeed();
const decreaseSpeed = () => window.go?.main?.App?.DecreaseSpeed();
const quitApp = () => window.go?.main?.App?.QuitApp();

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
