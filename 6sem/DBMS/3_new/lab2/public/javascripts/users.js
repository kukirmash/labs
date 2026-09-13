$(document).ready(function(){
    $.ajax({
        type: 'GET',
        url: '/api/users',
        dataType: 'JSON'
    }).done(function(response) {
        response.users.forEach(user => {
            $('#tbl_users').append(
                `<tr>
                    <td>${user.id}</td>
                    <td>${user.fio}</td>
                    <td>${user.login}</td>
                    <td>${user.role_label}</td>
                </tr>`
            )
        });
    });
});