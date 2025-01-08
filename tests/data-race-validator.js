import { wgsl_to_json } from "wgsl-to-json"
import { exec } from "node:child_process"

const faialDrfPath = "./faial-drf"
const faialDrfOptions = [
  "--all-dims",
  "--all-levels",
]

export async function validateDataRaceWgsl(shaderSource) {
  const wgslJsonString = await wgsl_to_json(shaderSource)
  
  return new Promise((resolve, reject) => {
    const baseCommand = `${faialDrfPath} ${faialDrfOptions.join(' ')} --json`;
    console.log("Running", `${baseCommand} <wgsl_json>`)

    // TODO: THIS IS SUPER UNSAFE!!!
    const fullCommand = `${baseCommand} '${wgslJsonString}'`;
    exec(fullCommand, (error, stdout, stderr) => {
      if (error) {
        reject(error)
      }
      if (stderr) {
        reject(stderr)
      }
      const result = JSON.parse(stdout)
      const areDrf = Object.fromEntries(
        result.kernels
        .map(k => [k.kernel_name, k.status === 'drf'])
      )
      console.log("Kernel DRF status", areDrf)
      const areAllDrf = Object.values(areDrf).every(v => v)
      console.log("All Kernels Data-Race Free?", areAllDrf)
      resolve(areAllDrf)
    })
  })
}
