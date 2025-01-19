let dev;

async function getDevice() {
    // get device
    dev = (await navigator.hid.requestDevice({filters: [
        { vendorId: 0xcafe, productId: 0x0069 },
    ]}))[0];
    await dev.open();

    // add receive w/ timeout support
    dev.inputQueue = [];
    dev.waiterQueue = [];
    dev.addEventListener("inputreport", (event) => {
        if (event.reportId != 0) { return; }
        if (dev.waiterQueue.length > 0) {
            const resolve = dev.waiterQueue.shift();
            resolve(event.data);
        } else {
            dev.inputQueue.push(event.data);
        }
    });
    dev.receiveReport = async function (timeout=1000) {
        if (this.inputQueue.length > 0) {
            return Promise.resolve(this.inputQueue.shift());
        }
        return new Promise((resolve, reject) => {
            const cb = (data) => {
                clearTimeout(timer);
                resolve(data);
            };
            const timer = setTimeout(() => {
                this.waiterQueue.splice(this.waiterQueue.indexOf(cb), 1);
                reject("Timeout waiting for input report");
            }, timeout);
            this.waiterQueue.push(cb);
        });
    };

    // add register ops
    dev.regWrite = async function (mms, reg, val) {
        const buffer = new ArrayBuffer(8);
        const view = new DataView(buffer);
        view.setUint8(0, 0x00);
        view.setUint8(1, mms);
        view.setUint16(2, reg, false);
        view.setUint32(4, val, false);
        await this.sendReport(0, new Uint8Array(buffer));
        const resp = await this.receiveReport();
        if (resp.getUint8(0) != 0) { throw "Request rejected"; }
    };
    dev.regRead = async function (mms, reg) {
        const buffer = new ArrayBuffer(4);
        const view = new DataView(buffer);
        view.setUint8(0, 0x01);
        view.setUint8(1, mms);
        view.setUint16(2, reg, false);
        await this.sendReport(0, new Uint8Array(buffer));
        const resp = await this.receiveReport();
        if (resp.getUint8(0) != 0) { throw "Request rejected"; }
        return resp.getUint32(1, false);
    };

    // TODO modify UI?
    const setPLCA = async function (enable=true, leader=true, node_id=0, node_cnt=8) {
        const MMS = 4;
        await dev.regWrite(MMS, 0x8002, leader ? 0x0003 : 0x0002);
        await dev.regWrite(MMS, 0xCA01, enable << 15);
        await dev.regWrite(MMS, 0xCA02, (node_cnt << 8) | (node_id << 0));
    };
    await setPLCA();
    // await setPLCA(enable=false);
}

document.getElementById("open").addEventListener("click", getDevice);
