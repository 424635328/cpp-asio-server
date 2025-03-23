const http = require('http'); // 引入 http 模块
const os = require('os'); // 引入 os 模块，用于获取 CPU 核心数
const URL = require('url').URL; // 引入 url 模块的 URL 类
const { performance } = require('perf_hooks'); // 引入 perf_hooks 模块，用于性能测量
const crypto = require('crypto'); // 引入 crypto 模块，用于生成随机字符串
const { parseArgs } = require('util'); // 引入 util 模块的 parseArgs 函数

// 默认配置对象
const DEFAULT_CONFIG = {
    targetUrl: 'http://example.com/', // 目标 URL
    concurrency: os.cpus().length, // 并发数，默认为 CPU 核心数
    requestTimeout: 3000, // 请求超时时间，单位毫秒
    usePost: false, // 是否使用 POST 请求
    requestBodySize: 512, // POST 请求体大小，单位字节
    maxRequests: 10000, // 最大请求数
    showResponseData: false, // 是否显示响应数据
    successStatusCodes: [200], // 成功状态码
    failedRequestThreshold: 0.8, // 失败请求阈值
    useHttp2: false, // 是否使用 HTTP/2
    workerCount: os.cpus().length, // Worker 线程数（未使用，但保留配置）
    userAgent: 'PerformanceTestAgent', // User-Agent
    connection: 'keep-alive', // Connection
    requestHeaders: {} // 请求头
};

// 获取配置函数，从命令行参数、环境变量或默认配置中获取
function getConfig() {
    // 定义命令行参数选项
    const options = {
        'target-url': { type: 'string' }, // 目标 URL
        'concurrency': { type: 'string' }, // 并发数
        'request-timeout': { type: 'string' }, // 请求超时时间
        'use-post': { type: 'boolean' }, // 是否使用 POST 请求
        'request-body-size': { type: 'string' }, // POST 请求体大小
        'max-requests': { type: 'string' }, // 最大请求数
        'show-response-data': { type: 'boolean' }, // 是否显示响应数据
        'success-status-codes': { type: 'string' }, // 成功状态码
        'failed-request-threshold': { type: 'string' }, // 失败请求阈值
        'use-http2': { type: 'boolean' }, // 是否使用 HTTP/2
        'worker-count': { type: 'string' }, // Worker 线程数
        'user-agent': { type: 'string' }, // User-Agent
        'connection': { type: 'string' }, // Connection
        'request-headers': { type: 'string' } // 请求头
    };

    // 解析命令行参数
    const { values, positionals } = parseArgs({ options, allowPositionals: true });
    const env = process.env; // 获取环境变量

    // 处理位置参数，如果存在，则将其作为最大请求数
    let maxRequests = DEFAULT_CONFIG.maxRequests;
    if (positionals.length > 0) {
        const positionalMaxRequests = parseInt(positionals[0]);
        if (!isNaN(positionalMaxRequests)) {
            maxRequests = positionalMaxRequests;
        }
    }

    // 优先使用命令行参数，其次是环境变量，最后是默认配置
    maxRequests = parseInt(values['max-requests']) || parseInt(env.MAX_REQUESTS) || maxRequests;


    const config =  {
        targetUrl: values['target-url'] || env.TARGET_URL || DEFAULT_CONFIG.targetUrl, // 目标 URL
        concurrency: parseInt(values['concurrency']) || parseInt(env.CONCURRENCY) || DEFAULT_CONFIG.concurrency, // 并发数
        requestTimeout: parseInt(values['request-timeout']) || parseInt(env.REQUEST_TIMEOUT) || DEFAULT_CONFIG.requestTimeout, // 请求超时时间
        usePost: values['use-post'] !== undefined ? values['use-post'] : (env.USE_POST === 'true' || DEFAULT_CONFIG.usePost), // 是否使用 POST 请求
        requestBodySize: parseInt(values['request-body-size']) || parseInt(env.REQUEST_BODY_SIZE) || DEFAULT_CONFIG.requestBodySize, // POST 请求体大小
        maxRequests: maxRequests, // 最大请求数
        showResponseData: values['show-response-data'] !== undefined ? values['show-response-data'] : (env.SHOW_RESPONSE_DATA === 'true' || DEFAULT_CONFIG.showResponseData), // 是否显示响应数据
        successStatusCodes: (values['success-status-codes'] || env.SUCCESS_STATUS_CODES || DEFAULT_CONFIG.successStatusCodes.join(',')) // 成功状态码
            .split(',') // 将字符串按逗号分隔成数组
            .map(code => parseInt(code.trim())), // 将数组中的每个元素转换为整数并去除空格
        failedRequestThreshold: parseFloat(values['failed-request-threshold']) || parseFloat(env.FAILED_REQUEST_THRESHOLD) || DEFAULT_CONFIG.failedRequestThreshold, // 失败请求阈值
        useHttp2: values['use-http2'] !== undefined ? values['use-http2'] : (env.USE_HTTP2 === 'true' || DEFAULT_CONFIG.useHttp2), // 是否使用 HTTP/2
        workerCount: parseInt(values['worker-count']) || parseInt(env.WORKER_COUNT) || DEFAULT_CONFIG.workerCount, // Worker 线程数
        userAgent: values['user-agent'] || env.USER_AGENT || DEFAULT_CONFIG.userAgent, // User-Agent
        connection: values['connection'] || env.CONNECTION || DEFAULT_CONFIG.connection, // Connection
        requestHeaders: {} // 请求头
    };

    // 处理请求头，从环境变量和命令行参数中获取
    try {
        let requestHeaders = { ...DEFAULT_CONFIG.requestHeaders }; // 从默认配置中复制请求头

        if (env.REQUEST_HEADERS) {
            requestHeaders = { ...requestHeaders, ...JSON.parse(env.REQUEST_HEADERS) }; // 从环境变量中解析请求头并合并
        }

        if (values['request-headers']) {
            requestHeaders = { ...requestHeaders, ...JSON.parse(values['request-headers']) }; // 从命令行参数中解析请求头并合并
        }
        config.requestHeaders = requestHeaders; // 设置请求头
    } catch (error) {
        console.error("Error parsing request headers:", error); // 打印解析请求头错误
    }
    return config; // 返回配置对象
}

const config = getConfig(); // 获取配置

// 从配置中提取变量
const TARGET_URL = config.targetUrl; // 目标 URL
const CONCURRENCY = config.concurrency; // 并发数
const REQUEST_TIMEOUT = config.requestTimeout; // 请求超时时间
const USE_POST = config.usePost; // 是否使用 POST 请求
const REQUEST_BODY_SIZE = config.requestBodySize; // POST 请求体大小
const MAX_REQUESTS = config.maxRequests; // 最大请求数
const SHOW_RESPONSE_DATA = config.showResponseData; // 是否显示响应数据
const SUCCESS_STATUS_CODES = config.successStatusCodes; // 成功状态码
const FAILED_REQUEST_THRESHOLD = config.failedRequestThreshold; // 失败请求阈值
const USE_HTTP2 = config.useHttp2; // 是否使用 HTTP/2
const WORKER_COUNT = config.workerCount; // Worker 线程数 (未使用)
const USER_AGENT = config.userAgent; // User-Agent
const CONNECTION = config.connection; // Connection
const REQUEST_HEADERS = config.requestHeaders; // 请求头

const parsedUrl = new URL(TARGET_URL); // 解析目标 URL
const HOST = parsedUrl.hostname; // 主机名
const PORT = parsedUrl.port || (parsedUrl.protocol === 'https:' ? 443 : 80); // 端口号
const PATH = parsedUrl.pathname; // 路径
const PROTOCOL = USE_HTTP2 ? require('http2') : (parsedUrl.protocol.startsWith('https') ? require('https') : require('http')); // 根据协议选择 http/https 模块

const keepAliveAgent = new PROTOCOL.Agent({ keepAlive: true }); // 创建 Keep-Alive Agent，用于连接复用

// 生成随机字符串，用于 POST 请求体
function generateRandomString(length) {
    return crypto.randomBytes(length).toString('hex').slice(0, length);
}

const requestBody = USE_POST ? generateRandomString(REQUEST_BODY_SIZE) : ""; // 生成 POST 请求体

// 发送 HTTP 请求的函数，返回 Promise
async function sendHttpRequest() {
    return new Promise((resolve, reject) => {
        const options = {
            hostname: HOST, // 主机名
            port: PORT, // 端口号
            path: PATH, // 路径
            method: USE_POST ? 'POST' : 'GET', // 请求方法
            timeout: REQUEST_TIMEOUT, // 超时时间
            headers: { // 请求头
                'Host': HOST,
                'Connection': CONNECTION,
                'User-Agent': USER_AGENT,
                ...(USE_POST ? { 'Content-Type': 'text/plain', 'Content-Length': Buffer.byteLength(requestBody) } : {}),
                ...REQUEST_HEADERS
            },
            agent: keepAliveAgent // 使用 Keep-Alive Agent
        };

        const req = PROTOCOL.request(options, (res) => {
            let responseData = ''; // 存储响应数据

            res.on('data', (chunk) => {
                responseData += chunk;
            });

            res.on('end', () => {
                if (SHOW_RESPONSE_DATA) {
                    console.log(`Status Code: ${res.statusCode}`);
                    console.log(`Response Data: ${responseData}`);
                }

                if (SUCCESS_STATUS_CODES.includes(res.statusCode)) {
                    resolve(true); // 成功
                } else {
                    resolve(false); // 失败
                }
            });
        });

        req.on('error', (err) => {
            console.warn('Request Error: ' + err.message);
            reject(err);
        });

        req.on('timeout', () => {
            console.warn('Request timed out');
            req.destroy(); // 销毁请求
            reject(new Error('Request timed out'));
        });

        if (USE_POST) {
            req.write(requestBody); // 写入请求体
        }
        req.end(); // 结束请求
    });
}

// 发送 HTTP/2 请求的函数，返回 Promise
async function sendHttp2Request() {
    return new Promise((resolve, reject) => {
        const client = PROTOCOL.connect(TARGET_URL, {
            rejectUnauthorized: false // 允许自签名证书
        });

        client.on('error', (err) => {
            console.error('HTTP/2 Client Error:', err);
            reject(err);
        });

        const req = client.request({
            ':path': PATH,
            ':method': USE_POST ? 'POST' : 'GET',
            'Host': HOST,
            'User-Agent': USER_AGENT,
            'Connection': CONNECTION,
            ...(USE_POST ? { 'Content-Type': 'text/plain', 'Content-Length': Buffer.byteLength(requestBody) } : {}),
            ...REQUEST_HEADERS

        });

        req.setTimeout(REQUEST_TIMEOUT, () => {
            console.warn('HTTP/2 Request timed out');
            req.close(); // 关闭请求
            client.close(); // 关闭客户端
            reject(new Error('HTTP/2 Request timed out'));
        });

        let responseData = ''; // 存储响应数据
        req.on('data', (chunk) => {
            responseData += chunk;
        });

        req.on('end', () => {
            if (SHOW_RESPONSE_DATA) {
                console.log(`Status Code: ${req.statusCode}`);
                console.log(`Response Data: ${responseData}`);
            }
            const statusCode = req.headers[':status']; // 获取状态码

            if (SUCCESS_STATUS_CODES.includes(statusCode)) {
                resolve(true); // 成功
            } else {
                resolve(false); // 失败
            }
            req.close(); // 关闭请求
            client.close(); // 关闭客户端
        });


        req.on('error', (err) => {
            console.warn('HTTP/2 Request Error: ' + err.message);
            req.close(); // 关闭请求
            client.close(); // 关闭客户端
            reject(err);
        });


        if (USE_POST) {
            req.write(requestBody); // 写入请求体
        }
        req.end(); // 结束请求
    });
}

const sendRequest = USE_HTTP2 ? sendHttp2Request : sendHttpRequest; // 根据是否使用 HTTP/2 选择请求函数

// 主函数
async function main() {
    const startTime = performance.now(); // 记录开始时间
    const maxRequests = config.maxRequests; // 最大请求数

    // 打印配置信息
    console.log(`开始性能测试:
    目标: ${TARGET_URL},
    并发: ${CONCURRENCY},
    总请求数: ${MAX_REQUESTS},
    HTTP/2: ${USE_HTTP2},
    请求超时: ${REQUEST_TIMEOUT}ms,
    使用 POST: ${USE_POST},
    请求体大小: ${REQUEST_BODY_SIZE} bytes,
    显示响应数据: ${SHOW_RESPONSE_DATA},
    成功状态码: ${SUCCESS_STATUS_CODES},
    失败率阈值: ${FAILED_REQUEST_THRESHOLD},
    User-Agent: ${USER_AGENT},
    Connection: ${CONNECTION},
    请求头: ${JSON.stringify(REQUEST_HEADERS)}`);

    let completedRequests = 0; // 完成的请求数
    let errors = 0; // 错误数
    let successfulRequests = 0; // 成功的请求数

    // 循环发送请求，直到达到最大请求数
    while (completedRequests < maxRequests) {
        try {
            const success = await sendRequest(); // 发送请求
            completedRequests++; // 完成的请求数加 1
            if (success) { // 如果请求成功
                successfulRequests++; // 成功的请求数加 1
            } else { // 如果请求失败
                errors++; // 错误数加 1
            }
        } catch (err) { // 捕获错误
            errors++; // 错误数加 1
        }
    }


    const endTime = performance.now(); // 记录结束时间
    const elapsedTime = (endTime - startTime) / 1000; // 计算运行时间

    console.log("性能测试完成");
    console.log(`总运行时间: ${elapsedTime.toFixed(1)} 秒`);
    console.log(`总成功请求数: ${successfulRequests}`);
    console.log(`总错误数: ${errors}`);
    console.log(`总完成请求数: ${completedRequests}`);

    const failureRate = errors / completedRequests; // 计算失败率
    if (failureRate > FAILED_REQUEST_THRESHOLD) { // 如果失败率超过阈值
        console.warn("警告：总失败率过高，可能服务器过载或存在问题。");
    }

    process.exit(0); // 退出进程
}


main(); // 运行主函数

console.log("\n提示: 为了模拟DDoS, 尝试同时运行多个客户端 (在不同的机器上)."); // 提示信息