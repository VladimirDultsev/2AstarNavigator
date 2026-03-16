const { spawn } = require('node:child_process');
const fs = require('fs').promises;
const { PathFinder } = require('./index.js'); 

async function findRoute() {
    const pathFinder = new PathFinder(
        './build/2AStar_Release', // Путь к собранному C++ модулю
        './output/output.geojson', // Путь к файлу, куда нужно записать маршрут
         './data/moscow_roads.geojson' // Путь к графу
    );

    try {
        pathFinder.start(); // запуск C++ модуля

        // Отправляем координаты и ждём сигнала готовности
        await pathFinder.getPath(
            55.760295, 37.450394,  // старт
            55.829925, 37.823384  // финиш
        );

        console.log('Маршрут найден');

        // Читаем файл с маршрутом
        const fileContent = await fs.readFile('./output/output.geojson', 'utf8');
        const routeData = JSON.parse(fileContent);

        // Выводим количество точек маршрута
        console.log('Точек в маршруте:', routeData.features[0].geometry.coordinates.length);

    } catch (error) {
        console.error('Ошибка:', error.message);
    } finally {
        await pathFinder.stop();
    }
}

findRoute();
