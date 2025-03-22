// beautify.js
document.addEventListener('DOMContentLoaded', function () {
    const body = document.body;
    const numParticles = 75; // 增加粒子数量

    // 粒子颜色
    const particleColors = ['#f48fb1', '#80deea', '#f06292', '#4dd0e1']; // 更多颜色选择

    // 创建粒子
    function createParticle() {
        const particle = document.createElement('div');
        particle.classList.add('particle');
        body.appendChild(particle);

        // 随机位置
        const x = Math.random() * 100;
        const y = Math.random() * 100;
        particle.style.left = `${x}vw`;
        particle.style.top = `${y}vh`;

        // 随机大小
        const size = Math.random() * 2 + 0.5; // 减小最大尺寸
        particle.style.width = `${size}px`;
        particle.style.height = `${size}px`;

        // 随机颜色
        const color = particleColors[Math.floor(Math.random() * particleColors.length)];
        particle.style.backgroundColor = color;

        // 随机动画持续时间 (加速)
        const animationDuration = Math.random() * 5 + 3; // 调整动画持续时间
        particle.style.animationDuration = `${animationDuration}s`;

        // 随机动画延迟
        const animationDelay = Math.random() * 2;
        particle.style.animationDelay = `-${animationDelay}s`;

        // 随机浮动方向
        const directionX = Math.random() > 0.5 ? 1 : -1;
        const directionY = Math.random() > 0.5 ? 1 : -1;
        particle.style.setProperty('--direction-x', directionX);
        particle.style.setProperty('--direction-y', directionY);

        return particle;
    }

    // 初始化粒子
    function initParticles() {
        for (let i = 0; i < numParticles; i++) {
            createParticle();
        }
    }

    initParticles();
});