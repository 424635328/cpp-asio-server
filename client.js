const http = require('http');
const os = require('os');

// 设置 HTTP Agent 的最大 Socket 连接数 (激进值)
http.globalAgent.maxSockets = Infinity; // 移除连接数限制

const HOST = '10.200.139.111';
const PORT = 8765;
const NUM_REQUESTS = Infinity; // 无限循环请求
const CONCURRENCY = 1000; // 激进的并发数
const REQUEST_TIMEOUT = 2000; // 缩短超时时间
const USE_POST = false; // 是否使用 POST 请求
const REQUEST_BODY_SIZE = 1024; // POST 请求体大小 (字节)

// 创建随机字符串 (用于 POST 请求体)
function generateRandomString(length) {
    let result = '';
    const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    const charactersLength = characters.length;
    for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength));
    }
    return result;
}

const requestBody = USE_POST ? generateRandomString(REQUEST_BODY_SIZE) : "";

async function sendRequest() {
    return new Promise((resolve, reject) => {
        const options = {
            hostname: HOST,
            port: PORT,
            path: '/',
            method: USE_POST ? 'POST' : 'GET',
            timeout: REQUEST_TIMEOUT,
            headers: {
                'Host': 'localhost',
                'Connection': 'keep-alive',
                'User-Agent': 'DDosTestAgent',
                'Content-Type': 'text/plain', // POST 请求需要设置
                'Content-Length': Buffer.byteLength(requestBody) // POST 请求需要设置
            }
        };

        const req = http.request(options, (res) => {
            // 移除数据处理
            res.on('data', (chunk) => {
                // 忽略响应数据， 不做任何处理， 减少客户端压力
            });
            res.on('end', () => {
                resolve();
            });
        });

        req.on('error', (err) => {
            // 降低错误日志级别
            console.warn('Request Error: ' + err.message);
            reject(err);
        });

        req.on('timeout', () => {
            console.warn('Request timed out');
            req.destroy();
            reject(new Error('Request timed out'));
        });

        if (USE_POST) {
            req.write(requestBody); // 发送请求体
        }
        req.end();
    });
}

async function flood(concurrency) {
    let activeRequests = 0;
    let completedRequests = 0;
    let errors = 0;

    const startTime = Date.now();

    const reportInterval = setInterval(() => {
        const elapsedTime = (Date.now() - startTime) / 1000;
        const requestsPerSecond = completedRequests / elapsedTime;
        console.log(
            `运行时间: ${elapsedTime.toFixed(1)} 秒, 完成请求: ${completedRequests}, 错误: ${errors}, RPS: ${requestsPerSecond.toFixed(2)}`
        );
    }, 1000);

    const queue = [];
    for (let i = 0; i < concurrency; i++) {
        queue.push(Promise.resolve());
    }

    let i = 0;
    while (true) { // 无限循环
        await queue[i % concurrency];
        queue[i % concurrency] = sendRequest()
            .then(() => {
                completedRequests++;
            })
            .catch((err) => {
                errors++;
            })
            .finally(() => {
                activeRequests--;
            });
        activeRequests++;
        i++;
    }
}

async function main() {
    const concurrency = 1000; // 大并发
    console.log(`开始DDoS测试: 无限请求, ${concurrency} 并发`);
    flood(concurrency);
}

main();

console.log("\n提示: 为了模拟DDoS, 尝试同时运行多个客户端 (在不同的机器上).");