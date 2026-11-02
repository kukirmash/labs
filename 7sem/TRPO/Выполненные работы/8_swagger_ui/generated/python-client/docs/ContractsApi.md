# excursion_api.ContractsApi

All URIs are relative to *https://api.excursion-bureau.ru/api/v1*

Method | HTTP request | Description
------------- | ------------- | -------------
[**create_contract**](ContractsApi.md#create_contract) | **POST** /contracts | Оформить договор на экскурсию (UC-04)


# **create_contract**
> ContractResponse create_contract(contract_input, idempotency_key=idempotency_key)

Оформить договор на экскурсию (UC-04)

Создаёт договор на выбранную экскурсию, формирует электронные
билеты участников и возвращает клиентский токен.

Повторный запрос с тем же значением заголовка Idempotency-Key
не создаёт второй договор, а возвращает результат первой операции.


### Example


```python
import excursion_api
from excursion_api.models.contract_input import ContractInput
from excursion_api.models.contract_response import ContractResponse
from excursion_api.rest import ApiException
from pprint import pprint

# Defining the host is optional and defaults to https://api.excursion-bureau.ru/api/v1
# See configuration.py for a list of all supported configuration parameters.
configuration = excursion_api.Configuration(
    host = "https://api.excursion-bureau.ru/api/v1"
)


# Enter a context with an instance of the API client
with excursion_api.ApiClient(configuration) as api_client:
    # Create an instance of the API class
    api_instance = excursion_api.ContractsApi(api_client)
    contract_input = {"excursionId":"e3b0c442-98fc-4c14-9b9f-1f5d3a7b8c21","participants":[{"lastName":"Петров","firstName":"Пётр","middleName":"Петрович","passportSeries":"9701","passportNumber":"123456"}],"contactEmail":"petrov@example.ru","contactPhone":"+79001234567","consentPersonalData":true} # ContractInput | Данные для оформления договора.
    idempotency_key = UUID('6f1c2b0e-7d3a-4c9e-9f1a-2b3c4d5e6f70') # UUID | Ключ идемпотентности повторного запроса (UUID). (optional)

    try:
        # Оформить договор на экскурсию (UC-04)
        api_response = api_instance.create_contract(contract_input, idempotency_key=idempotency_key)
        print("The response of ContractsApi->create_contract:\n")
        pprint(api_response)
    except Exception as e:
        print("Exception when calling ContractsApi->create_contract: %s\n" % e)
```



### Parameters


Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **contract_input** | [**ContractInput**](ContractInput.md)| Данные для оформления договора. | 
 **idempotency_key** | **UUID**| Ключ идемпотентности повторного запроса (UUID). | [optional] 

### Return type

[**ContractResponse**](ContractResponse.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json, application/problem+json

### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**201** | Договор создан, электронные билеты сформированы. |  * Location - Ссылка на созданный договор. <br>  |
**400** | Некорректный запрос (ошибка формата JSON). |  -  |
**409** | Конфликт состояния (нет свободных мест или экскурсия отменена). |  -  |
**422** | Ошибка валидации входных данных. |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

