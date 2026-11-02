# ContractInput

Запрос на оформление договора.

## Properties

Name | Type | Description | Notes
------------ | ------------- | ------------- | -------------
**excursion_id** | **UUID** | Идентификатор экскурсии. | 
**participants** | [**List[Participant]**](Participant.md) | Участники экскурсии (от 1 до 20 человек). | 
**contact_email** | **str** | E-mail для отправки копии документов. | 
**contact_phone** | **str** | Контактный телефон клиента. | [optional] 
**consent_personal_data** | **bool** | Согласие на обработку персональных данных. | 

## Example

```python
from excursion_api.models.contract_input import ContractInput

# TODO update the JSON string below
json = "{}"
# create an instance of ContractInput from a JSON string
contract_input_instance = ContractInput.from_json(json)
# print the JSON string representation of the object
print(ContractInput.to_json())

# convert the object into a dict
contract_input_dict = contract_input_instance.to_dict()
# create an instance of ContractInput from a dict
contract_input_from_dict = ContractInput.from_dict(contract_input_dict)
```
[[Back to Model list]](../README.md#documentation-for-models) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to README]](../README.md)


