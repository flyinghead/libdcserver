#!/usr/bin/env node

export {}

import process from 'node:process';
import fs from 'fs';
import pug from 'pug';

interface Status {
    gameId: string;
    name: string | undefined;
    thumbnail: string | undefined;
    timestamp: number;
    playerCount: number | undefined;
    gameCount: number | undefined;
    online: boolean | undefined;
}

interface GameInfo {
    name: string;
    thumbnail: string;
}

function logError(error: any, prefix: string | undefined)
{
    const start = prefix !== undefined ? `${prefix}: ` : '';
    if (error instanceof Error)
        console.error(start + error.message);
    else
        console.error(start + error);
}

function loadStatus(path: string): Status[] | undefined {
    var json:string;
    try {
        json = fs.readFileSync(path, 'utf8');
    } catch (error) {
        logError(error, `Error reading file ${path}`);
        return undefined;
    }
    try {
        const status:Status[] = JSON.parse(json);
        return status;
    } catch (error) {
        logError(error, `Invalid json in ${path}`);
        return undefined;
    }
}

try {
    const statusDir = process.argv[2] ?? "/var/lib/dcnet/status";
    const destDir = process.argv[3] ?? "/var/www/dcnet/status";
    if (!statusDir || !destDir) {
        console.error(`Usage: ${process.argv[1]} [<status dir> [<dest dir>]]`);
        process.exit(1);
    }
    const gameInfos: { [k: string]: GameInfo } = JSON.parse(fs.readFileSync('/usr/local/share/dcnet/games.json', 'utf8'));
    const now = Date.now() / 1000;
    var statusArray:Status[] = [];
    const files = fs.readdirSync(statusDir);
    files.forEach(file => {
        console.log('Loading ' + file);
        var allStatus = loadStatus(statusDir + '/' + file);
        if (allStatus === undefined)
           return;
        if (!Array.isArray(allStatus)) {
            console.error(`Content of ${file} isn't an array`);
            return;
        }
        // Filter out non working games
        allStatus = allStatus.filter(status => status.gameId !== 'culdcept' 
            && status.gameId !== 'yakyuunet' 
            && status.gameId != 'bomberman' 
            && status.gameId != 'propeller');
        allStatus.forEach(status => {
            console.log(`game ${status.gameId}, timestamp ${status.timestamp}, players ${status.playerCount}`);
            status.name = gameInfos[status.gameId]?.name;
            status.thumbnail = gameInfos[status.gameId]?.thumbnail;
            // Expect an update every 5 min. Assume the server is offline after 6 min.
            status.online = now - status.timestamp < 6 * 60;
            if (!status.online) {
                status.playerCount = 0;
                status.gameCount = 0;
            }
        });
        statusArray = statusArray.concat(allStatus);
    });
    statusArray = statusArray.sort((g1, g2) => {
        // put active games on top
        const playerDiff = (g2.playerCount ? 1 : 0) - (g1.playerCount ? 1 : 0);
        if (playerDiff != 0)
            return playerDiff;
        else
            return g1.name!.localeCompare(g2.name!);
    });
    fs.writeFileSync(destDir + '/' + "status.json", JSON.stringify(statusArray, undefined, 4));
    const html = pug.renderFile('status.pug', {
        statusArray: statusArray
    });
    fs.writeFileSync(destDir + '/' + "status.html", html);
} catch (error) {
    logError(error, 'Error');
    process.exit(1);
}