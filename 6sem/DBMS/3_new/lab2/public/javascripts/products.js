function createProduct() {
    let data = {
        sku: $('#prodSku').val(),
        label: $('#prodLabel').val(),
        base_price: $('#prodPrice').val(),
        brand: $('#prodBrand').val(),
        category: $('#prodCategory').val(),
        color: $('#prodColor').val()
    };

    if(!data.sku || !data.label || !data.base_price) {
        alert('Пожалуйста, заполните обязательные поля со звездочкой (*)');
        return;
    }

    $.ajax({
        type: 'POST',
        url: '/products/create',
        data: data,
        dataType: 'JSON'
    }).done(function(response) {
        if (response.msg === '') {
            alert('Товар успешно создан в PostgreSQL, MongoDB и счетчик инициализирован в Redis!');
            window.location.reload();
        } else {
            alert(response.msg);
        }
    });
}

function deleteProduct(id) {
    if(!confirm('Вы уверены, что хотите удалить товар?')) return;

    $.ajax({
        type: 'POST',
        url: '/products/delete/' + id,
        dataType: 'JSON'
    }).done(function(response) {
        if (response.msg === '') {
            window.location.reload();
        } else {
            alert(response.msg);
        }
    });
}