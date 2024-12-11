import { describe, it } from 'node:test'
import * as assert from 'node:assert'
import AoLoader from '@permaweb/ao-loader'
import fs from 'fs'
import WeaveDrive from "@permaweb/weavedrive"
// import WeaveDrive from "./weavedrive.js"
import { validateDataRaceWgsl } from './data-race-validator.js'
import gpu from '@kmamal/gpu'
const instance = gpu.create([ 'verbose=1' ])
const adapter = await instance.requestAdapter()

const env = getEnv();
const wasm = fs.readFileSync('./process.wasm')
const options = {
    format: "wasm32-unknown-emscripten-webgpu-draft_2024_11_30",
    applyMetering: true,
    mode: "test",
    ARWEAVE: "https://arweave.net",
    WeaveDrive,
    ...env,
    dataRaceValidator: validateDataRaceWgsl,
    // unsafe: true,
}
describe('sqlite', async () => {
    const handle = await AoLoader(wasm, options)
    let Memory = null;
    it('Create DB', {
        timeout: 1000 * 60 * 10,
    }, async () => {
        let device = await adapter.requestDevice()
        for (let i = 0; i < 100; i++) {
            console.log("=====================================")
            console.log(`Iteration: ${i}`)
            console.log("=====================================")

            const opts = {
                preinitializedWebGPUDevice: device,
                outputMemory: false,
            }

            // load handler
            const result = await handle(
                Memory, 
                getEval(`
                local sokoldemo = require('lsokoldemo')
                local s = sokoldemo.demo()
                local Hex = require(".crypto.util.hex")
                return Hex.stringToHex(s)
                `),
                env,
                opts,
            );
            Memory = result.Memory;
            console.log('Memory', Memory?.length);

            // assert.ok(true)
            
            const hexString = result.Output.data
            const binaryData = Buffer.from(hexString, 'hex')
            console.log(`Hex: ${hexString.length}, Binary: ${binaryData.length}`)
            fs.writeFileSync(`output/frame_${i}.png`, binaryData, {
                encoding: 'binary'
            })

            if (!options.unsafe) {
                // Recreate the device for safety
                try {
                    device.destroy()
                } catch (e) {
                    console.error(e)
                }
                // Recreating devices too quickly can cause a crash
                const requestDevice = adapter.requestDevice()
                console.log("Waiting for 1 second before creating a new device")
                await new Promise((resolve) => setTimeout(resolve, 1000))
                device = await requestDevice;
            }
        }
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
        Module: {
            Tags: [{ name: "Extension", value: "Weave-Drive" }]
        },
        Spawn: {
            Tags: [{ name: "Name", value: "TEST_SPAWN_OWNER" }],
        },
        Process: {
            Id: "AOS",
            Owner: "FOOBAR",

            Tags: [{ name: "Name", value: "TEST_PROCESS_OWNER" }],
        },
    };
}