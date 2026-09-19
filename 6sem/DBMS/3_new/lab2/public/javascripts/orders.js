$(document).ready(function(){
    $('#create_order').click(function(e){
        $('#create_order_popup').show()
    })
    $('#create_order_popup_close').click(function(e){
        $('#create_order_popup').hide()
    })
    $('#cancel_create_order').click(function(e){
        $('#create_order_popup').hide()
    })

    $('#submit_create_order').click(function(e){
        e.preventDefault()
        let data = {
            label:    $('#inpLabel').val(),
            id_client: $('#inpClient').val(),
            amount: $('#inpAmount').val(),
        }

        $.ajax({
            type: 'POST',
            data: data,
            url: '/orders/create',
            dataType: 'JSON'
        }).done(function( response ) {
            if (response.msg === '') {
                alert('Заказ создан')
                window.location.reload()
            }
            else {
                alert(response.msg)
            }
        });
    })
});

// --- ДОБАВЛЕННЫЕ ФУНКЦИИ ДЛЯ СТРАНИЦЫ ПРОСМОТРА ЗАКАЗА ---

// Смена статуса
function changeStatus(orderId) {
    let data = {
        id_status: $('#new_status').val() // Считываем новый статус из выпадающего списка
    };

    $.ajax({
        type: 'POST',
        url: '/orders/' + orderId + '/status',
        data: data,
        dataType: 'JSON'
    }).done(function(response) {
        if (response.msg === '') {
            window.location.reload();
        } else {
            alert(response.msg);
        }
    });
}

// Добавление элемента
function addItem(orderId) {
    let data = {
        id_product: $('#item_product_id').val() // Считываем выбранный ID товара
    };

    $.ajax({
        type: 'POST',
        url: '/orders/' + orderId + '/items',
        data: data,
        dataType: 'JSON'
    }).done(function(response) {
        if (response.msg === '') {
            window.location.reload();
        } else {
            alert(response.msg);
        }
    });
}

// Проведение оплаты
function addPayment(orderId) {
    let data = {
        id_order: orderId,
        id_payment_type: $('#payment_type').val(),
        amount: $('#payment_amount').val()
    };
    if(!data.amount) return alert("Укажите сумму");

    $.post('/payments/create', data, function(res) {
        if (!res.msg) location.reload();
        else alert(res.msg);
    });
}