const { spawn } = require('node:child_process');

class PathFinder {
    constructor(executablePath, outputPath, graphPath) {
        this.executablePath = executablePath;
        this.outputPath = outputPath;
        this.graphPath = graphPath;
        this.proc = null;
    }

    start() {
        if (this.proc) {// Если процесс уже существует, выкидываем ошибку
            throw new Error('Process already running');
        }
        // Запускаем исполняемый файл по пути executablePath с аргументами в виде пути до графа и пути до файла куда нужно записать путь
        this.proc = spawn(this.executablePath, [this.outputPath, this.graphPath]);
        this.proc.on('error', (err) => {
            console.error('Failed to start process:', err);
            this.proc = null;
        });
        this.proc.on('close', (code) => {
            console.log(`Process exited with code ${code}`);
            this.proc = null;
        });
        return this.proc;
    }

    async getPath(lat1, lon1, lat2, lon2, timeout = 10000) {
        if (!this.proc) {
            throw new Error('Process not started');
        }
        return new Promise((resolve, reject) => {
            let response = '';
            let timeoutId;
            
            // Если получили информацию, завершаемся успешно
            const onData = (data) => {
                response += data.toString();
                cleanup();
                resolve(response.trim());
            };
            
            // Если получили ошибку, завершаемся с ошибкой
            const onError = (err) => {
                cleanup();
                reject(new Error(`Process error: ${err.message}`));
            };
            
            // Если через timeout миллисекунд ответ не придёт, завершаемся с ошибкой
            timeoutId = setTimeout(() => {
                cleanup();
                reject(new Error('Request timeout'));
            }, timeout);
            
            const cleanup = () => {
                clearTimeout(timeoutId);
                this.proc.stdout.removeListener('data', onData);
                this.proc.stderr.removeListener('data', onError);
            }
            
            this.proc.stdout.on('data', onData);
            this.proc.stderr.on('data', onError);
            // Отправляем в поток stdin информацию о точках
            this.proc.stdin.write(`${lat1} ${lon1} ${lat2} ${lon2}\n`);
        });
    }

    async stop() {
        if (this.proc) {
            return new Promise((resolve) => {
                this.proc.on('close', () => resolve());
                this.proc.kill('SIGTERM'); // Пробуем завершить процесс мягко
                setTimeout(() => {
                    // Если не получилось - завершаем жёстко
                    if (this.proc) {
                        this.proc.kill('SIGKILL');
                    }
                }, 3000);
            });
        }
    }
}
