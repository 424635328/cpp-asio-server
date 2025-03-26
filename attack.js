const http = require('http');
const os = require('os');
const URL = require('url').URL;
const { performance } = require('perf_hooks');
const crypto = require('crypto');
const { parseArgs } = require('util');
const async = require('async');
const winston = require('winston'); // 日志库
const { createHash } = require('crypto');
const http2 = require('http2');

// 配置日志
const logger = winston.createLogger({
    level: 'info',
    format: winston.format.combine(
        winston.format.timestamp({
            format: 'YYYY-MM-DD HH:mm:ss'
        }),
        winston.format.printf(({ timestamp, level, message, ...meta }) => {
            return `${timestamp} [${level.toUpperCase()}] ${message} ${Object.keys(meta).length ? JSON.stringify(meta) : ''}`;
        })
    ),
    transports: [
        new winston.transports.Console()
    ]
});

// 常量
const DEFAULT_MAX_SOCKETS = os.cpus().length * 4;
const DEFAULT_REQUEST_BODY = 'DEFAULT_REQUEST_BODY';

http.globalAgent.maxSockets = DEFAULT_MAX_SOCKETS; // 合理设置 maxSockets

const DEFAULT_CONFIG = {
    // targetUrl: 'http://127.0.0.1:8765/',
    // targetUrl: 'http://example.com/',
    targetUrl: 'http://bilibili.com/',
    concurrency: os.cpus().length * 4,
    requestTimeout: 3000,
    usePost: false,
    requestBodySize: 512,
    maxRequests: 2000,
    showResponseData: false,
    successStatusCodes: [200],
    failedRequestThreshold: 0.8,
    useHttp2: false,
    workerCount: os.cpus().length,
    userAgent: 'PerformanceTestAgent',
    requestHeaders: {}
};

function getConfig() {
    const options = {
        'target-url': { type: 'string' },
        'concurrency': { type: 'string' },
        'request-timeout': { type: 'string' },
        'use-post': { type: 'boolean' },
        'request-body-size': { type: 'string' },
        'max-requests': { type: 'string' },
        'show-response-data': { type: 'boolean' },
        'success-status-codes': { type: 'string' },
        'failed-request-threshold': { type: 'string' },
        'use-http2': { type: 'boolean' },
        'worker-count': { type: 'string' },
        'user-agent': { type: 'string' },
        'request-headers': { type: 'string' }
    };

    const { values, positionals } = parseArgs({ options, allowPositionals: true });
    const env = process.env;

    let maxRequests = DEFAULT_CONFIG.maxRequests;
    if (positionals.length > 0) {
        const positionalMaxRequests = parseInt(positionals[0]);
        if (!isNaN(positionalMaxRequests)) {
            maxRequests = positionalMaxRequests;
        }
    }

    maxRequests = parseInt(values['max-requests']) || parseInt(env.MAX_REQUESTS) || maxRequests;

    const config = {
        targetUrl: values['target-url'] || env.TARGET_URL || DEFAULT_CONFIG.targetUrl,
        concurrency: parseInt(values['concurrency']) || parseInt(env.CONCURRENCY) || DEFAULT_CONFIG.concurrency,
        requestTimeout: parseInt(values['request-timeout']) || parseInt(env.REQUEST_TIMEOUT) || DEFAULT_CONFIG.requestTimeout,
        usePost: values['use-post'] !== undefined ? values['use-post'] : (env.USE_POST === 'true' || DEFAULT_CONFIG.usePost),
        requestBodySize: parseInt(values['request-body-size']) || parseInt(env.REQUEST_BODY_SIZE) || DEFAULT_CONFIG.requestBodySize,
        maxRequests: maxRequests,
        showResponseData: values['show-response-data'] !== undefined ? values['show-response-data'] : (env.SHOW_RESPONSE_DATA === 'true' || DEFAULT_CONFIG.showResponseData),
        successStatusCodes: (values['success-status-codes'] || env.SUCCESS_STATUS_CODES || DEFAULT_CONFIG.successStatusCodes.join(','))
            .split(',')
            .map(code => parseInt(code.trim())),
        failedRequestThreshold: parseFloat(values['failed-request-threshold']) || parseFloat(env.FAILED_REQUEST_THRESHOLD) || DEFAULT_CONFIG.failedRequestThreshold,
        useHttp2: values['use-http2'] !== undefined ? values['use-http2'] : (env.USE_HTTP2 === 'true' || DEFAULT_CONFIG.useHttp2),
        workerCount: parseInt(values['worker-count']) || parseInt(env.WORKER_COUNT) || DEFAULT_CONFIG.workerCount,
        userAgent: values['user-agent'] || env.USER_AGENT || DEFAULT_CONFIG.userAgent,
        requestHeaders: {}
    };

    try {
        let requestHeaders = { ...DEFAULT_CONFIG.requestHeaders };

        if (env.REQUEST_HEADERS) {
            requestHeaders = { ...requestHeaders, ...JSON.parse(env.REQUEST_HEADERS) };
        }

        if (values['request-headers']) {
            requestHeaders = { ...requestHeaders, ...JSON.parse(values['request-headers']) };
        }
        config.requestHeaders = requestHeaders;
    } catch (error) {
        logger.error("Error parsing request headers:", { error: error.message }); // 使用 logger
    }

    // Validate Config
    validateConfig(config);

    return config;
}

function validateConfig(config) {
    if (typeof config.targetUrl !== 'string' || !isValidURL(config.targetUrl)) {
        throw new Error('Invalid target URL.');
    }

    if (!Number.isInteger(config.concurrency) || config.concurrency <= 0) {
        throw new Error('Concurrency must be a positive integer.');
    }

    if (!Number.isInteger(config.requestTimeout) || config.requestTimeout <= 0) {
        throw new Error('Request timeout must be a positive integer.');
    }

    if (!Number.isInteger(config.requestBodySize) || config.requestBodySize < 0) {
        throw new Error('Request body size must be a non-negative integer.');
    }

    if (!Number.isInteger(config.maxRequests) || config.maxRequests <= 0) {
        throw new Error('Max requests must be a positive integer.');
    }

    if (typeof config.failedRequestThreshold !== 'number' || config.failedRequestThreshold < 0 || config.failedRequestThreshold > 1) {
        throw new Error('Failed request threshold must be a number between 0 and 1.');
    }

    if (!Array.isArray(config.successStatusCodes) || config.successStatusCodes.length === 0) {
        throw new Error('Success status codes must be a non-empty array.');
    }

    config.successStatusCodes.forEach(code => {
        if (!Number.isInteger(code) || code <= 0) {
            throw new Error('Success status codes must be an array of positive integers.');
        }
    });
}


function isValidURL(string) {
    try {
        new URL(string);
        return true;
    } catch (_) {
        return false;
    }
}

const config = getConfig();

const TARGET_URL = config.targetUrl;
const CONCURRENCY = config.concurrency;
const REQUEST_TIMEOUT = config.requestTimeout;
const USE_POST = config.usePost;
const REQUEST_BODY_SIZE = config.requestBodySize;
const MAX_REQUESTS = config.maxRequests;
const SHOW_RESPONSE_DATA = config.showResponseData;
const SUCCESS_STATUS_CODES = config.successStatusCodes;
const FAILED_REQUEST_THRESHOLD = config.failedRequestThreshold;
const USE_HTTP2 = config.useHttp2;
const WORKER_COUNT = config.workerCount;
const USER_AGENT = config.userAgent;
const REQUEST_HEADERS = config.requestHeaders;

const parsedUrl = new URL(TARGET_URL);
const HOST = parsedUrl.hostname;
const PORT = parsedUrl.port || (parsedUrl.protocol === 'https:' ? 443 : 80);
const PATH = parsedUrl.pathname;
const PROTOCOL = USE_HTTP2 ? require('http2') : (parsedUrl.protocol.startsWith('https') ? require('https') : require('http'));

const keepAliveAgent = USE_HTTP2 ? undefined : new PROTOCOL.Agent({ keepAlive: true });

// 使用 SHA256 生成请求体哈希，用于避免传输过大的随机字符串
function generateRequestBodyHash(length) {
    const randomString = crypto.randomBytes(length).toString('hex').slice(0, length);
    const hash = createHash('sha256').update(randomString).digest('hex');
    return hash; // 使用hash替代body
}

const requestBody = USE_POST ? generateRequestBodyHash(REQUEST_BODY_SIZE) : "";

let requestIdCounter = 0; // 请求ID计数器

async function sendHttpRequest(statusCodeCounts, totalSentBytes, totalReceivedBytes) {
    const requestId = ++requestIdCounter; // 生成请求ID
    return new Promise((resolve, reject) => {
        const options = {
            hostname: HOST,
            port: PORT,
            path: PATH,
            method: USE_POST ? 'POST' : 'GET',
            timeout: REQUEST_TIMEOUT,
            headers: {
                'Host': HOST,
                'User-Agent': USER_AGENT,
                ...(USE_POST ? { 'Content-Type': 'text/plain', 'Content-Length': Buffer.byteLength(requestBody) } : {}),
                ...REQUEST_HEADERS
            },
            agent: keepAliveAgent
        };

        const startTime = performance.now();
        let req;

        try {
            req = PROTOCOL.request(options, (res) => {
                let responseData = '';
                let receivedBytes = 0;

                res.on('data', (chunk) => {
                    responseData += chunk;
                    receivedBytes += chunk.length;
                });

                res.on('end', () => {
                    const endTime = performance.now();
                    const elapsedTime = (endTime - startTime) / 1000;
                    logger.info(`Request ${requestId}: HTTP Request completed`, {
                        elapsedTime: elapsedTime.toFixed(3),
                        statusCode: res.statusCode
                    });

                    statusCodeCounts[res.statusCode] = (statusCodeCounts[res.statusCode] || 0) + 1;
                    totalReceivedBytes.value += receivedBytes;

                    if (SHOW_RESPONSE_DATA) {
                        console.log(`Status Code: ${res.statusCode}`);
                        console.log(`Response Data: ${responseData}`);
                    }

                    if (SUCCESS_STATUS_CODES.includes(res.statusCode)) {
                        resolve(true);
                    } else {
                        resolve(false);
                    }
                });
            });

            req.on('error', (err) => {
                const endTime = performance.now();
                const elapsedTime = (endTime - startTime) / 1000;
                logger.warn(`Request ${requestId}: HTTP Request failed`, {
                    elapsedTime: elapsedTime.toFixed(3),
                    error: err.message
                });
                reject(err);
            });

            req.on('timeout', () => {
                logger.warn(`Request ${requestId}: Request timed out`);
                req.destroy();
                reject(new Error('Request timed out'));
            });

            if (USE_POST) {
                req.write(requestBody);
                totalSentBytes.value += Buffer.byteLength(requestBody);
            }
            req.end();
        } catch (error) {
            logger.error(`Request ${requestId}: Error during HTTP request creation/execution`, { error: error.message });
            reject(error);
        }
    });
}

// HTTP/2 客户端的连接池
const http2Clients = new Map();

// 获取或创建 HTTP/2 客户端
function getHttp2Client(targetUrl) {
    if (http2Clients.has(targetUrl)) {
        return http2Clients.get(targetUrl);
    }

    const client = http2.connect(targetUrl, {
        rejectUnauthorized: false, // 注意安全隐患，仅测试环境使用
        maxSessionMemory: 1000, // Add to limit total memory usage
    });

    client.on('error', (err) => {
        logger.error('HTTP/2 Client Error:', { error: err.message });  // 使用 logger
        http2Clients.delete(targetUrl);
    });

    client.on('close', () => {
        logger.info('HTTP/2 Client closed'); // 使用 logger
        http2Clients.delete(targetUrl);
    });

    client.setTimeout(REQUEST_TIMEOUT, () => {
        client.close();
        logger.warn("HTTP/2 Client timed out and closed.");
    });

    http2Clients.set(targetUrl, client);
    return client;
}

async function sendHttp2Request(statusCodeCounts, totalSentBytes, totalReceivedBytes) {
    const requestId = ++requestIdCounter; // 生成请求ID
    return new Promise((resolve, reject) => {
        const client = getHttp2Client(TARGET_URL);

        const headers = {
            ':path': PATH,
            ':method': USE_POST ? 'POST' : 'GET',
            'host': HOST,
            'User-Agent': USER_AGENT,
            ...REQUEST_HEADERS
        };

        const startTime = performance.now();

        const req = client.request(headers);
        let responseData = '';
        let statusCode;
        let receivedBytes = 0;

        req.setTimeout(REQUEST_TIMEOUT, () => {
            logger.warn(`Request ${requestId}: HTTP/2 Request timed out`);
            req.destroy(new Error('Request timed out'));
            reject(new Error('HTTP/2 Request timed out'));
        });

        req.on('data', (chunk) => {
            responseData += chunk;
            receivedBytes += chunk.length;
        });

        req.on('headers', (headers) => {
            statusCode = headers[':status'];
        });

        req.on('end', () => {
            const endTime = performance.now();
            const elapsedTime = (endTime - startTime) / 1000;

            if (statusCode !== undefined) {
                logger.info(`Request ${requestId}: HTTP/2 Request completed`, {
                    elapsedTime: elapsedTime.toFixed(3),
                    statusCode: statusCode
                });
            } else {
                logger.warn(`Request ${requestId}: HTTP/2 Request completed without status code`);
            }

            statusCodeCounts[statusCode] = (statusCodeCounts[statusCode] || 0) + 1;
            totalReceivedBytes.value += receivedBytes;

            if (SHOW_RESPONSE_DATA) {
                console.log(`Status Code: ${statusCode}`);
                console.log(`Response Data: ${responseData}`);
            }

            if (statusCode && SUCCESS_STATUS_CODES.includes(parseInt(statusCode))) {
                resolve(true);
            } else {
                resolve(false);
            }
            req.close();
        });

        req.on('error', (err) => {
            logger.error(`Request ${requestId}: HTTP/2 Request Error:`, { error: err.message });
            req.close();
            reject(err);
        });

        if (USE_POST) {
            req.write(requestBody);
            totalSentBytes.value += Buffer.byteLength(requestBody);
        }
        req.end();
    });
}

// 使用一个闭包来封装 sendRequest 函数
function createSendRequest(statusCodeCounts, totalSentBytes, totalReceivedBytes) {
    return USE_HTTP2
        ? () => sendHttp2Request(statusCodeCounts, totalSentBytes, totalReceivedBytes)
        : () => sendHttpRequest(statusCodeCounts, totalSentBytes, totalReceivedBytes);
}

// Helper function to calculate median
function median(arr) {
    const mid = Math.floor(arr.length / 2);
    const nums = [...arr].sort((a, b) => a - b);
    return arr.length % 2 !== 0 ? nums[mid] : (nums[mid - 1] + nums[mid]) / 2;
}

// Helper function to calculate percentile
function percentile(arr, p) {
    if (arr.length === 0) return 0;
    const nums = [...arr].sort((a, b) => a - b);
    const index = (p / 100) * (nums.length - 1);
    const lower = Math.floor(index);
    const upper = Math.ceil(index);
    const frac = index - lower;
    return nums[lower] + (nums[upper] - nums[lower]) * frac;
}

async function main() {
    const startTime = performance.now();
    const maxRequests = config.maxRequests;

    logger.info(`开始性能测试`, {
        targetUrl: TARGET_URL,
        concurrency: CONCURRENCY,
        maxRequests: MAX_REQUESTS,
        useHttp2: USE_HTTP2,
        requestTimeout: REQUEST_TIMEOUT,
        usePost: USE_POST,
        requestBodySize: REQUEST_BODY_SIZE,
        showResponseData: SHOW_RESPONSE_DATA,
        successStatusCodes: SUCCESS_STATUS_CODES,
        userAgent: USER_AGENT,
        requestHeaders: REQUEST_HEADERS
    });

    let completedRequests = 0;
    let errors = 0;
    let successfulRequests = 0;
    let responseTimes = []; // 用于记录响应时间
    const statusCodeCounts = {}; // 用于记录状态码分布
    const totalSentBytes = { value: 0 }; // 用于记录总发送字节数
    const totalReceivedBytes = { value: 0 }; // 用于记录总接收字节数

    // 使用 async.queue 来控制并发
    const q = async.queue(async (task) => {
        let startTime;
        try {
            startTime = performance.now();
            const success = await sendRequest();
            const endTime = performance.now();
            const elapsedTime = (endTime - startTime) / 1000;
            responseTimes.push(elapsedTime); // 记录响应时间

            if (success) {
                successfulRequests++;
            } else {
                errors++;
            }
        } catch (err) {
            errors++;
            logger.error("Request execution error:", { error: err.message });
        } finally {
            completedRequests++;
        }
    }, CONCURRENCY);

    const sendRequest = createSendRequest(statusCodeCounts, totalSentBytes, totalReceivedBytes);

    // 添加任务到队列
    for (let i = 0; i < maxRequests; i++) {
        q.push({});
    }

    // 等待队列完成
    await q.drain();

    const endTime = performance.now();
    const elapsedTime = (endTime - startTime) / 1000;

    // 计算响应时间统计信息
    const totalResponseTime = responseTimes.reduce((sum, time) => sum + time, 0);
    const averageResponseTime = totalResponseTime / completedRequests;
    const maxResponseTime = responseTimes.length > 0 ? Math.max(...responseTimes) : 0;
    const minResponseTime = responseTimes.length > 0 ? Math.min(...responseTimes) : 0;
    const medianResponseTime = responseTimes.length > 0 ? median(responseTimes) : 0;
    const p90ResponseTime = responseTimes.length > 0 ? percentile(responseTimes, 90) : 0;
    const requestsPerSecond = (completedRequests / elapsedTime).toFixed(2);
    const successRate = ((successfulRequests / completedRequests) * 100).toFixed(2);

    logger.info(`
--------------------- 性能测试完成 ---------------------
总运行时间: ${elapsedTime.toFixed(1)} 秒
总请求数: ${completedRequests}
成功请求数: ${successfulRequests}
错误数: ${errors}
请求成功率: ${successRate}%
请求吞吐量 (RPS): ${requestsPerSecond}
--------------------- 数据传输 (字节) ---------------------
总发送字节数: ${totalSentBytes.value}
总接收字节数: ${totalReceivedBytes.value}
--------------------- 响应时间 (秒) ---------------------
平均值: ${averageResponseTime.toFixed(3)}
中位数: ${medianResponseTime.toFixed(3)}
90 分位值: ${p90ResponseTime.toFixed(3)}
最大值: ${maxResponseTime.toFixed(3)}
最小值: ${minResponseTime.toFixed(3)}
--------------------- 状态码分布 ---------------------
${Object.entries(statusCodeCounts).map(([code, count]) => `${code}: ${count}`).join('\n')}
-------------------------------------------------------
`);

    const failureRate = errors / completedRequests;
    if (failureRate > FAILED_REQUEST_THRESHOLD) {
        logger.warn("警告：总失败率过高，可能服务器过载或存在问题。", { failureRate: failureRate.toFixed(2) });
    }

    // 关闭所有的 HTTP/2 客户端
    http2Clients.forEach(client => client.close());

    process.exit(0);
}

try {
    main();
} catch (error) {
    logger.error("Fatal error during execution:", { error: error.message });
    process.exit(1);
}

console.log("\n提示: 为了模拟DDoS, 尝试同时运行多个客户端 (在不同的机器上).");