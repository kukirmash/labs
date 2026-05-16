$(document).ready(function() {
    console.log("Layout.js загружен и готов к работе!");

    $('#Login').click(function(e) {
        e.preventDefault();
        console.log("Нажата ссылка Войти");
        $('#login_popup').css('display', 'flex');
    });

    $('#submit_login').click(function(e) {
        e.preventDefault();
        console.log("Нажата кнопка отправки формы");
        
        let data = {
            login: $('#inpLogin').val(),
            password: $('#inpPassword').val()
        };

        $.ajax({
            type: 'POST',
            url: '/api/auth/login',
            data: data,
            dataType: 'JSON'
        }).done(function(response) {
            if (response.msg === '') {
                window.location.reload(); 
            } else {
                alert(response.msg); 
            }
        });
    });

    // 4. Кнопка "Выйти"
    $('#Logout').click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'POST',
            url: '/api/auth/logout',
            dataType: 'JSON'
        }).done(function(response) {
            window.location.reload();
        });
    });
});