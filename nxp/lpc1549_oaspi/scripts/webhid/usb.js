const DEVICE_MAP = {
    0xbc0189a1: "NCN26010",
    0x0283bc91: "ADIN1100",
};

async function openDevice() {
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
    dev.setPLCA = async function (enable=true, leader=true, node_id=0, node_cnt=8) {
        const MMS = 4;
        await dev.regWrite(MMS, 0x8002, leader ? 0x0003 : 0x0002);
        await dev.regWrite(MMS, 0xCA01, enable << 15);
        await dev.regWrite(MMS, 0xCA02, (node_cnt << 8) | (node_id << 0));
    };

    // modify UI
    document.getElementById("device").onclick = closeDevice;
    document.getElementById("device").innerHTML = "close";
    document.getElementById("device").style.backgroundColor = "#80ff80";
    document.getElementById("reg").style.display = "";
    document.getElementById("plca").style.display = "";

    let id = await dev.regRead(0, 0x0001);
    let pn = DEVICE_MAP[id] || "unknown";
    document.getElementById("reg-id").innerHTML = "id: 0x" + id.toString(16) + ` (${pn})`;
}

async function closeDevice() {
    // close device
    await dev.close();
    await dev.forget();

    // modify UI
    document.getElementById("device").onclick = openDevice;
    document.getElementById("device").innerHTML = "open";
    document.getElementById("device").style.backgroundColor = "#ff8080";
    document.getElementById("reg").style.display = "none";
    document.getElementById("plca").style.display = "none";
}

async function regWrite() {
    const mms = parseInt(document.getElementById("reg-write-mms").value);
    const reg = parseInt(document.getElementById("reg-write-reg").value);
    const val = parseInt(document.getElementById("reg-write-val").value);
    try {
        if (isNaN(mms) || isNaN(reg) || isNaN(val)) { throw new Error("invalid input!"); }
        document.getElementById("reg-write-result").innerHTML = "waiting...";
        await dev.regWrite(mms, reg, val);
        document.getElementById("reg-write-result").innerHTML = "success!";
    } catch {
        document.getElementById("reg-write-result").innerHTML = "fail!";
    }
}

async function regRead() {
    const mms = parseInt(document.getElementById("reg-read-mms").value);
    const reg = parseInt(document.getElementById("reg-read-reg").value);
    try {
        if (isNaN(mms) || isNaN(reg)) { throw new Error("invalid input!"); }
        document.getElementById("reg-read-val").innerHTML = "waiting...";
        document.getElementById("reg-read-val").innerHTML = "val: 0x" + (await dev.regRead(mms, reg)).toString(16);
    } catch {
        document.getElementById("reg-read-val").innerHTML = "fail!";
    }
}

async function setPLCA() {
    const enable   = document.getElementById("plca-enable").checked;
    const leader   = document.getElementById("plca-leader").checked;
    const node_id  = parseInt(document.getElementById("plca-node-id").value);
    const node_cnt = parseInt(document.getElementById("plca-node-cnt").value);
    try {
        if (isNaN(node_id) || isNaN(node_cnt)) { throw new Error("invalid input!"); }
        document.getElementById("plca-result").innerHTML = "waiting...";
        await dev.setPLCA(enable, leader, node_id, node_cnt);
        document.getElementById("plca-result").innerHTML = "success!";
    } catch {
        document.getElementById("plca-result").innerHTML = "fail!";
    }
}

// setup UI
document.getElementById("device").onclick = openDevice;
document.getElementById("device").style.backgroundColor = "#ff8080";
document.getElementById("reg").style.display = "none";
document.getElementById("plca").style.display = "none";
document.getElementById("reg-write-do").onclick = regWrite;
document.getElementById("reg-read-do").onclick = regRead;
document.getElementById("plca-do").onclick = setPLCA;
