const { PathFinder } = require('./index.js');

async function findRoute() {
    const pathFinder = new PathFinder(
        './build/2AStar_Release',
        './output/output.geojson',
        './data/moscow_roads.geojson'
    );
    
    try {
        pathFinder.start();// Запускаем исполняемый файл с алгоритмом 
        
        // Ваши координаты
        const result = await pathFinder.getPath(
            55.752220, 37.615555,  
            55.733768, 37.588588   
        );
        
        console.log('Маршрут найден');
        console.log('Точек в маршруте:', result.features[0].geometry.coordinates.length);
        
    } catch (error) {
        console.error('Ошибка:', error.message);
    } finally {
        await pathFinder.stop();
    }
}

findRoute();
