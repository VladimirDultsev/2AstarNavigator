function bulidGraph(executablePath, outputPath, graphPath){
    const { spawn } = require('node:child_process');
    var args = [outputPath, graphPath];
    let child = spawn(executablePath, args);
    return child;
}
function getPath(proc, lat1, lon1, lat2, lon2){
    proc.stdin.write(lat1 + " "  + lon1 + " " + lat2 + " " + lon2 + "\n");
    return new Promise((resolve) => {
        console.log("waiting...");
        proc.stdout.on('data', (data) => {
            console.log("data received");
            resolve();
        });
    });
}

function exit(proc){
    proc.kill('SIGKILL');
}
//55.692446, 37.893894
//55.856142, 37.347318
let lat1 = 55.888799, lon1 =37.565719, lat2 = 55.609171, lon2 = 37.571662;
const executablePath = '/Users/vladimir/CLionProjects/2AStar_Release/cmake-build-debug/2AStar_Release';
const outputPath = '/Users/vladimir/CLionProjects/2AStar_Release/output.geojson';
const graphPath = '/Users/vladimir/CLionProjects/2A*/moscow_roads.geojson';

const proc = bulidGraph(executablePath, outputPath, graphPath);
getPath(proc, lat1, lon1, lat2, lon2).then(() => exit(proc));
