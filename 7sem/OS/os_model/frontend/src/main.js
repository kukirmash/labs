// Элементы вывода статистики
const pcElement = document.getElementById('pc-val');
const speedElement = document.getElementById('speed-val');

// Элементы кнопок
const btnIncrease = document.getElementById('btn-increase');
const btnDecrease = document.getElementById('btn-decrease');
const btnQuit = document.getElementById('btn-quit');

// Слушаем события от Go ядра
window.runtime.EventsOn("update_stats", (pc, speed) => {
    pcElement.innerText = pc;
    speedElement.innerText = speed.toFixed(2);
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