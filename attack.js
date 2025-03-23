const http = require('http');
const os = require('os');
const URL = require('url').URL;

// 设置 HTTP Agent 的最大 Socket 连接数 (激进值)
http.globalAgent.maxSockets = Infinity; // 移除连接数限制

const TARGET_URL = 'http://127.0.0.1:8765/'; // 目标网址
const CONCURRENCY = 1000; // 激进的并发数
const REQUEST_TIMEOUT = 2000; // 缩短超时时间
const USE_POST = false; // 是否使用 POST 请求
const REQUEST_BODY_SIZE = 1024; // POST 请求体大小 (字节)
const MAX_REQUESTS = 1000; // 设置一个默认的最大请求数
const SHOW_RESPONSE_DATA = true; // 是否显示响应数据

// 从URL中提取主机名、端口和路径
const parsedUrl = new URL(TARGET_URL);
const HOST = parsedUrl.hostname;
const PORT = parsedUrl.port || (parsedUrl.protocol === 'https:' ? 443 : 80); // 自动检测端口
const PATH = parsedUrl.pathname;
const PROTOCOL = parsedUrl.protocol.startsWith('https') ? require('https') : require('http'); // 根据协议选择模块

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
            path: PATH,
            method: USE_POST ? 'POST' : 'GET',
            timeout: REQUEST_TIMEOUT,
            headers: {
                'Host': HOST, //  使用目标主机名，而不是 localhost
                'Connection': 'keep-alive',
                'User-Agent': 'DDosTestAgent',
                'Content-Type': 'text/plain', // POST 请求需要设置
                'Content-Length': Buffer.byteLength(requestBody) // POST 请求需要设置
            }
        };

        const req = PROTOCOL.request(options, (res) => { // 使用https或http模块
            let responseData = ''; // 用于存储响应数据

            res.on('data', (chunk) => {
                responseData += chunk; // 收集响应数据
            });

            res.on('end', () => {
                //  输出响应状态码和数据
                if (SHOW_RESPONSE_DATA) {
                  console.log(`Status Code: ${res.statusCode}`);
                  console.log(`Response Data: ${responseData}`); // 显示响应数据
                }
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

async function flood(concurrency, maxRequests) {
    let activeRequests = 0;
    let completedRequests = 0;
    let errors = 0;
    let requestCount = 0;

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
    while (requestCount < maxRequests) {
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
        requestCount++;
    }
    clearInterval(reportInterval);
    console.log("请求完成，退出程序")
}

async function main() {
    const concurrency = 1000; // 大并发
    const maxRequests = parseInt(process.argv[2]) || MAX_REQUESTS; // 从命令行参数获取请求数
    console.log(`开始DDoS测试:  ${concurrency} 并发, 目标: ${TARGET_URL}, 最大请求数：${maxRequests}`);
    await flood(concurrency, maxRequests);
}

main();

console.log("\n提示: 为了模拟DDoS, 尝试同时运行多个客户端 (在不同的机器上).");