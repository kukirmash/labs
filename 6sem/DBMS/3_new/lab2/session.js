const md5 = require('md5')
const crypto = require('crypto')

exports.sessions = {}

// ИСПРАВЛЕНИЕ: добавлено слово async
exports.login = async function(req, login, pass) {
    let cookies = req.cookies
    let secret = cookies['app_user']

    // ИСПРАВЛЕНИЕ: используем oneOrNone, чтобы не было ошибки, если логин не найден
    let user = await req.db.oneOrNone('SELECT * FROM users WHERE login = $1', login)

    if (user && (user.pass == md5(pass))) {
        let secret = 'secret';
        let hash = crypto.createHmac('sha256', secret)
                   .update(login)
                   .digest('hex');

        let cookie = login + '--' + hash;
        exports.sessions[login] = {
            active:    1,
            timestamp: new Date().getTime(),
            user: user // Сохраняем данные пользователя в сессии
        }
        return cookie;
    }
    return 0;
}

exports.auth = function(req) {
    let cookies = req.cookies
    let secret = cookies['app_user']

    if (!secret) return {}
    let res = secret.split('--');
    if(!res.length) return {}
    
    let session = exports.sessions[res[0]]
    if (!session) return {};
    
    let current_timestamp = new Date().getTime()
    if (!session.active || ((current_timestamp - session.timestamp) > 43200*1000)) {
        return 0;
    }
    return session;
}

exports.logout = function(login) {
    exports.sessions[login] = {}
}