# ContractResponse

Результат оформления договора.

## Properties

Name | Type | Description | Notes
------------ | ------------- | ------------- | -------------
**contract** | [**Contract**](Contract.md) |  | 
**client_token** | **str** | Токен доступа клиента к своим договорам. | 

## Example

```python
from excursion_api.models.contract_response import ContractResponse

# TODO update the JSON string below
json = "{}"
# create an instance of ContractResponse from a JSON string
contract_response_instance = ContractResponse.from_json(json)
# print the JSON string representation of the object
print(ContractResponse.to_json())

# convert the object into a dict
contract_response_dict = contract_response_instance.to_dict()
# create an instance of ContractResponse from a dict
contract_response_from_dict = ContractResponse.from_dict(contract_response_dict)
```
[[Back to Model list]](../README.md#documentation-for-models) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to README]](../README.md)


