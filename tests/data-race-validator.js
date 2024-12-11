import { wgsl_to_json } from "wgsl-to-json"
import { exec } from "node:child_process"

const pathToFaialDrfExe = "./faial-drf"

export async function validateDataRaceWgsl(shaderSource) {
  const wgslJsonString = await wgsl_to_json(shaderSource)
  
  return new Promise((resolve, reject) => {
    // TODO: THIS IS SUPER UNSAFE!!!
    exec(`${pathToFaialDrfExe} --json '${wgslJsonString}'`, (error, stdout, stderr) => {
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
