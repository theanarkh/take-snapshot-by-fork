const addon = require('..')

class MainClass {}
const obj = new MainClass();
global.buf = Buffer.alloc(1024 * 1024  * 1024)
const xxxx = {}
for (let i = 0; i < 100000000; i++) {
    xxxx[i] = i
}
setInterval(() => {
    obj;
    xxxx;
}, 2000)

setTimeout(() => {
    const t1 = Date.now();
    addon.takeSnapshotByFork(`./${process.pid}.heapsnapshot`)
    console.log(Date.now()-t1)
}, 1000);