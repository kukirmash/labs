// Элементы вывода статистики ЦП
const pcElement = document.getElementById('pc-val');
const speedElement = document.getElementById('speed-val');
const currentProcElement = document.getElementById('current-proc-val');

// Элементы вывода статистики Памяти
const memUsedElement = document.getElementById('mem-used');
const memFreeElement = document.getElementById('mem-free');
const memPercentElement = document.getElementById('mem-percent');
const totalMemory = 1000; // Базовое значение из Go-модели

// Элементы кнопок
const btnIncrease = document.getElementById('btn-increase');
const btnDecrease = document.getElementById('btn-decrease');
const btnQuit = document.getElementById('btn-quit');

const processList = document.getElementById('process-list');

// Слушаем события от Go ядра
window.runtime.EventsOn("update_stats", (pc, speed, procs) => {
    pcElement.innerText = pc;
    speedElement.innerText = speed.toFixed(2);
    
    let usedMem = 0;
    let currentActiveProc = '--';
    
    // Очистка и перерисовка таблицы процессов
    processList.innerHTML = '';
    procs.forEach((proc, index) => {
        // Подсчет занятой памяти
        if (proc.state !== 'Отсутствует') {
            usedMem += proc.size;
        }

        // Поиск выполняющегося процесса для панели ЦП
        if (proc.state === 'Выполняется') {
            currentActiveProc = `ID: ${proc.id}`;
        }

        const tr = document.createElement('tr');
        
        // Подсветка активных и пустых строк
        if (proc.state === 'Отсутствует') {
            tr.style.opacity = '0.3';
        }

        tr.innerHTML = `
            <td style="padding: 8px 0;">[${index}]</td>
            <td>${proc.id === 0 ? '-' : proc.id}</td>
            <td>${proc.size === 0 ? '-' : proc.size}</td>
            <td class="font-mono">${proc.pc}</td>
            <td>${proc.prior}</td>
            <td><span class="badge ${proc.state === 'Отсутствует' ? '' : 'active'}">${proc.state}</span></td>
        `;
        processList.appendChild(tr);
    });

    // Обновление метрик ЦП
    currentProcElement.innerText = currentActiveProc;

    // Обновление метрик Памяти
    const freeMem = totalMemory - usedMem;
    const percentMem = ((usedMem / totalMemory) * 100).toFixed(1);
    
    memUsedElement.innerText = usedMem;
    memFreeElement.innerText = freeMem;
    memPercentElement.innerText = `${percentMem}%`;
});

// Функции-обертки для вызовов Go
const increaseSpeed = () => window.go.main.App.IncreaseSpeed();
const decreaseSpeed = () => window.go.main.App.DecreaseSpeed();
const quitApp = () => window.go.main.App.QuitApp();

// Привязка кликов мыши
btnIncrease.addEventListener('click', increaseSpeed);
btnDecrease.addEventListener('click', decreaseSpeed);
btnQuit.addEventListener('click', quitApp);

// Привязка аппаратных прерываний (горячие клавиши)
window.addEventListener('keydown', (e) => {
    if (e.key === '=' || e.key === '+') {
        increaseSpeed();
        simulateClick(btnIncrease);
    } else if (e.key === '-') {
        decreaseSpeed();
        simulateClick(btnDecrease);
    } else if (e.key === 'Escape') {
        quitApp();
        simulateClick(btnQuit);
    }
});

// Визуальная подсветка кнопки при нажатии с клавиатуры
function simulateClick(btn) {
    btn.style.transform = 'scale(0.95)';
    setTimeout(() => btn.style.transform = '', 150);
}