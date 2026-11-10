# СУТ — синтаксически управляемая трансляция

Реализация LR-анализатора по таблице разбора из `LR_Table.xml` (генератор таблиц)
и семантических действий A2–A32 из рабочего файла.

## Модули

| Файл | Назначение |
|---|---|
| `SutTypes.h` | `LRElType`, `LRElem`, `TypeInfo`, `Attr` — общие структуры |
| `LRTable.h/.cpp` | загрузка `LR_Table.xml` (pugixml), столбцы, состояния, `LexCodeLR` |
| `SymbolTable.h/.cpp` | таблица символов: основная таблица сканера + таблицы полей записей, память данных, временные, константы |
| `Tac.h/.cpp` | трехадресный код: `Quad`, `Gen`, метки/переходы |
| `SemanticActions.h/.cpp` | процедуры действий A2–A32 |
| `SutParser.h/.cpp` | алгоритм LR-разбора из примера отчёта: стек состояний, стек атрибутов, `PredToken` |

## Алгоритм (как в примере)

```
ClearStack(AttrSt); ClearStack(ParserSt)
Push(1, ParserSt)
Token := Scaner(); PredToken := Token
Sym := LexCodeLR[Token.Code]
while ElemLR.ElType <> lrStop:
    State := StackTop(ParserSt)
    ElemLR := LRTbl[State, Sym]
    case ElType:
      lrErr:    Syntax_Error(State)
      lrShift:  Push(ElPar, ParserSt)
                if Sym — терминал:
                    PredToken := Token; Token := Scaner();
                    Sym := LexCodeLR[Token.Code]
                else Sym := LexCodeLR[Token.Code]
      lrReduct: for i := 1 to ElPar: Pop(ParserSt)
                Sym := ElemLR.Left
                if ElTab.Act <> 0: Semantic_Action(Act, PredToken)
```

`PredToken` хранит последний прочитанный терминал: действия A2, A4, A5, A9, A12,
A13, A15, A17, A19, A20, A21, A22, A25 берут из него атрибут (`id.pnt`, `num.pnt`,
код операции `add`/`mult`/`comp`). Терминалы в стек атрибутов не кладутся.

## Действия

| Действие | Продукция / момент | Что делает |
|---|---|---|
| A2 | `M2` после `id` в `TypeDef` | проверка повторного объявления, `Push(id.pnt)` |
| A3 | конец `TypeDef` | `AddType(id.pnt, catTypeName, Type.type)` |
| A4 | конец `VarDecl` | проверка и `AddType(id.pnt, catVarName, Type.type)` |
| A5 | `Type → id` | проверка, что `id` — имя типа; `Push(type)` |
| A6 | конец `StructType` | `p := Pop(ST); Record(p)`; восстановление `offset` из `SO` |
| A9 | `Field → Type id` | проверка поля, `AddField(Top(ST), id.pnt.Lex, Type.type)`; `offset += width` |
| A11 | конец `AssignStmt` | проверка типов, `Gen(opAss, Expr.addr, -, Var.addr)` |
| A12 | `Var → id` | проверка переменной; `Push(type)`, `Push(addr)` |
| A13 | `M5` после `add` | `Push(add.op)` |
| A14 | `MathExpr → add M5 Term` | унарный `+`/`-`; для `-` временная и `Gen(opSub, 0, Term.addr, tmp)` |
| A15 | `M6` после `add` | `Push(add.op)` |
| A16 | `MathExpr → MathExpr add M6 Term` | проверка типов, временная, `Gen(op, ...)` |
| A17 | `M7` после `mult` | `Push(mult.op)` |
| A18 | `MultTerm → MultTerm mult M7 Term` | проверка типов, временная, `Gen(op, ...)` |
| A19 | `Factor → num` | `Push(type)`, `Push(num.pnt)`; константе выделяется адрес |
| A20 | `Factor → true` | `Push(typeLogical)`, `Push(ConstTrue)` |
| A21 | `Factor → false` | `Push(typeLogical)`, `Push(ConstFalse)` |
| A22 | `StructAccess → Var . id` | проверка записи, `FindField`, адрес поля `base.addr + field.addr` |
| A23 | `M3` после `do` | `Push(nextinstr)` — метка начала тела цикла |
| A24 | конец `CtrlStmt` | `GenIf(Expr.addr, nextinstr+2)`, `GenGoto(M.instr)` |
| A25 | `M4` после `comp` | `Push(comp.op)` |
| A26 | `Expr → SimpleExpr comp M4 SimpleExpr` | проверка типов, временная, `Gen(comp, ...)`; для логических допустимы `==`, `!=` |
| A27 | `OrExpr → Term or Term` | проверка логических, `Gen(opOr, ...)` |
| A28 | `OrExpr → OrExpr or Term` | то же |
| A29 | `AndTerm → Factor and Factor` | проверка логических, `Gen(opAnd, ...)` |
| A30 | `AndTerm → AndTerm and Factor` | то же |
| A31 | `Factor → not Factor` | проверка логического, `Gen(opNot, ...)` |
| A32 | `M1` после `struct` | `NewTbl()`, `Push(ST)`, `Push(offset, SO)`, `offset := 0` |

Действия A1 (имя программы), A7, A8 (список полей) и A10 в грамматике отсутствуют:
имя программы — ключевое слово, его категория `catProgName` выставляется при запуске
разбора; поля записи сразу добавляются в отдельную таблицу (A9).

## Ошибки

Коды соответствуют табл. 5 отчёта: 1 — повторное объявление, 2 — не имя типа,
3 — не переменная, 4 — несовместимость типов, 5 — не объявлено, 6 — операнд
арифметики не числовой, 7 — операнд логики не логический, 8 — не запись,
9 — поле не найдено.

## Таблица разбора

`LR_Table.xml` читается `LRTable::Load`. Ожидается:

- `Columns/Neterms/Column` и `Columns/Terms/Column` с `ColNum` и `Lexeme`;
- `Row[@NSost]` с `ElemTR[@ColNum, @ElType, @ElPar, @Left, @Act]`;
- `Left` — **имя** нетерминала (в файле генератора), поэтому оно переводится в номер столбца;
- `Act` — `A2`…`A32` (пусто — действия нет).

`ElType`: 0 — ошибка, 1 — перенос (в `ElPar` номер состояния), 2 — свертка
(в `ElPar` длина правой части), 3 — останов.
