# Contract

Оформленный договор.

## Properties

Name | Type | Description | Notes
------------ | ------------- | ------------- | -------------
**id** | **UUID** |  | 
**number** | **str** | Номер договора. | 
**excursion_id** | **UUID** |  | 
**total_price** | **float** | Итоговая стоимость договора, ₽. | 
**status** | **str** |  | 
**created_at** | **datetime** |  | 
**payment_deadline** | **datetime** | Срок оплаты договора. | 
**participants** | [**List[Participant]**](Participant.md) |  | 
**tickets** | [**List[Ticket]**](Ticket.md) |  | 

## Example

```python
from excursion_api.models.contract import Contract

# TODO update the JSON string below
json = "{}"
# create an instance of Contract from a JSON string
contract_instance = Contract.from_json(json)
# print the JSON string representation of the object
print(Contract.to_json())

# convert the object into a dict
contract_dict = contract_instance.to_dict()
# create an instance of Contract from a dict
contract_from_dict = Contract.from_dict(contract_dict)
```
[[Back to Model list]](../README.md#documentation-for-models) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to README]](../README.md)


