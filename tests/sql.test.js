

import { describe, it } from 'node:test'
import * as assert from 'node:assert'
import AoLoader from '@permaweb/ao-loader'
import fs from 'fs'

const wasm = fs.readFileSync('./process.wasm')
const options = { format: "wasm64-unknown-emscripten-draft_2024_02_15", applyMetering: true }
describe('sqlite', async () => {
    const handle = await AoLoader(wasm, options)
    let Memory = null;
    it('Create DB', async () => {

        // load handler
        const result = await handle(Memory, getEval(`
            local sokol_demo = require('lsokol_demo')
            return 'YAY'
            `), getEnv());
        Memory = result.Memory;
        
        console.log(result)
        // console.log(result.Output.data)
        // console.log(result.GasUsed)

        assert.ok(true)
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