/**
 * 
 * Take care of extension commands!
 * 
**/

// Imports
import { commands, ExtensionContext } from "vscode";
import { deactivate, enableServerCycle, manageServerLifecycle } from "./server";
import { info, Section } from "./channel";

// Commands
const server = {
    stop: commands.registerCommand('juggernyaut.server.stop', async () => {
        info(Section.SERVER, "Deactivating the Juggernyaut language server...")
        enableServerCycle(false);
        await deactivate();
    }),
    resume: commands.registerCommand('juggernyaut.server.resume', async () => {
        info(Section.SERVER, "Resuming the Juggernyaut language server cycle...")
        enableServerCycle(true);
        await manageServerLifecycle();
    })
};

export function register(context: ExtensionContext) {
    context.subscriptions.push(server.stop, server.resume);
}
