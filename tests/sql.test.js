import { describe, it } from 'node:test'
import * as assert from 'node:assert'
import AoLoader from '@permaweb/ao-loader'
import fs from 'fs'
import gpu from '@kmamal/gpu'
const instance = gpu.create([ 'verbose=1' ])
const adapter = await instance.requestAdapter()
const device = await adapter.requestDevice()

const wasm = fs.readFileSync('./process.wasm')
const options = { format: "wasm32-unknown-emscripten-webgpu-draft_2024_11_30", applyMetering: true, preinitializedWebGPUDevice: device }
describe('sqlite', async () => {
    const handle = await AoLoader(wasm, options)
    let Memory = null;
    it('Create DB', async () => {

        // load handler
        const result = await handle(Memory, getEval(`
            local sokoldemo = require('lsokoldemo')
            local s = sokoldemo.demo()
            local Hex = require(".crypto.util.hex")
            return Hex.stringToHex(s)
            `), getEnv());
        Memory = result.Memory;
        
        console.log(result)
        // console.log(result.Output.data)
        // console.log(result.GasUsed)

        assert.ok(true)
        
        const hexString = result.Output.data
        const binaryData = Buffer.from(hexString, 'hex')
        console.log(`Hex: ${hexString.length}, Binary: ${binaryData.length}`)
        // trim to 13363
        fs.writeFileSync('out.png', binaryData, {
            encoding: 'binary'
        })

        await device.queue.onSubmittedWorkDone()
        device.destroy()
    });
});


function getEval(expr) {
    return {
        Target: "AOS",
        From: "FOOBAR",
        Owner: "FOOBAR",

        Module: "FOO",
        Id: "1",

        "Block-Height": "1000",
        Timestamp: Date.now(),
        Tags: [{ name: "Action", value: "Eval" }],
        Data: expr,
    };
}

function getEnv() {
    return {
        Process: {
            Id: "AOS",
            Owner: "FOOBAR",

            Tags: [{ name: "Name", value: "TEST_PROCESS_OWNER" }],
        },
    };
}