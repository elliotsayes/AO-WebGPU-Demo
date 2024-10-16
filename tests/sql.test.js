

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
            local sqlite = require('lsqlite3')
            local db = sqlite.open('test.db')
            db:exec('CREATE TABLE numbers(num1,num2)');
            db:close();
            db = nil

            collectgarbage()
            print(collectgarbage('count'))
            return ''
            `), getEnv());
        Memory = result.Memory;

        console.log(result.Output.data)
        console.log(result.GasUsed)

        assert.ok(true)
    });

    it('Insert Data', async () => {

        // load handler
        const result = await handle(Memory, getEval(`
            local sqlite = require('lsqlite3')
            local db = sqlite.open('test.db')
            for i=1,10000 do
                db:exec('INSERT INTO numbers VALUES ('..i..','..(i*2)..')')
            end
            db:close();
            db = nil

            collectgarbage()
            print(collectgarbage('count'))

            return ''
        `), getEnv());
        Memory = result.Memory;

        console.log(result.Output.data)
        console.log(result.GasUsed)

        assert.ok(true)
    });

    it('Select Data', async () => {
        const result = await handle(Memory, getEval(`
            local sqlite = require('lsqlite3')
            local db = sqlite.open('test.db')
            t = {}
            for a in db:nrows('select * from numbers') do table.insert(t, a) end
            print(#t)
            db:close();
            db = nil
            t = nil
            collectgarbage()
            print(collectgarbage('count'))

            return ''
        `), getEnv())
        Memory = result.Memory;

        console.log(result.Output.data)
        console.log(result.GasUsed)

        assert.ok(true)
    });

    it.skip('Math', async () => {
        const result = await handle(Memory, getEval(`
            local sqlite = require('lsqlite3')
            local db = sqlite.open('test.db')

            local po = db:exec('select power(2,2)');
            print(po)
            return ''
        `), getEnv())
        Memory = result.Memory;

        console.log(result.Output.data)
        console.log(result.GasUsed)
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