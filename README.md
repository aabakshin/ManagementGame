# Игра "Менеджмент"
Данная игра была предложена фирмой Avalon Hill Company для обучения основам управления предприятием. Ч.Уэзерелл отметил чрезвычайную привлекательность этой игры в качестве упражнения для
программистов. В этом репозитории находится моя реализация этой игры на чистом Си. Некоторые части программы также реализованы на C++ без использования каких-либо сторонних библиотек, контейнеров STL и т.д.
## Состав проекта
Проект сделан по клиент-серверной архитектуре. Клиентом может быть как реальный игрок, так и компьютер, стратегия поведения которого задаётся скриптовыми текстовыми файлами на специально созданном для него языке.
Все скрипты должны находиться в директории `game_scripts`. Краткое описание скриптового языка будет представлено далее.
### /
- `binaries`: Содержит исполняемые модули и их исходные коды, а также Makefile'ы для сборки проекта
- `game_scripts`: Содержит скрипты для интерпретации игроком-компьютером
- `includes`: Содержит заголовочные файлы модулей для всего проекта
- `libs`: Содержит исходные коды и объектные файлы модулей для всего проекта 
- `tests`: Содержит файлы и исходные тексты для тестирования некоторых частей проекта(например, контейнеров)
### /binaries
- `bot_mg`: Содержит исходник, бинарник и Makefile для сборки клиента-компьютера
- `client`: Содержит исходник, бинарник и Makefile для сборки клиента-игрока
- `server`: Содержит исходник, бинарник и Makefile для сборки сервера
## Запуск проекта
Проект собирался под Linux Mint 20.3, версия ядра Linux: 5.4.0-148-generic, архитектура: x86-64
Для запуска проекта необходимо:
- Установить SSH-сервер:
	```
	$ sudo apt update
	$ sudo apt upgrade
	```

	```
	$ sudo apt install openssh-server
	```

	```
	$ systemctl status ssh
	```
	Если всё настроено правильно, то должна быть надпись о том, что сервер работает

- Настроить ssh-туннель с github-сервером:
	1. Создание нового ключа SSH и привязка к ssh-agent:
	```
	https://docs.github.com/ru/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent
	```
	2. Добавление нового ключа SSH в учётную запись GitHub:
	```
	https://docs.github.com/ru/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account
	```
	3. Проверка работы SSH-соединения:
	```
	$ ssh -T git@github.com
	```

- Клонировать репозиторий:
	```
	git clone git@github.com:aabakshin/ManagementGame.git
	```

- Собрать бинарник сервера: Перейти в директорию `/binaries/server/`. Ввести команду `make server`
- Собрать бинарник клиента-игрока: Перейти в директорию `/binaries/client/`. Ввести команду `make client`
- Собрать бинарник клиента-компьютера(если нужно подключить ботов к игре): Перейти в директорию `/binaries/bot_mg/`. Ввести команду `make bot_mg`
- Далее запускается один экземпляр сервера: `./server 7777`, порт указывается любой свободный
- Далее запускаются несколько экземпляров игроков: `./client 192.168.50.128 7777`, в параметрах указывается адрес сервера и порт
- Также можно добавлять ботов, для этого перед стартом игры(в лобби), создаются экземпляры ботов: `./bot_mg 192.168.50.128 7777 ../game_scripts/example.txt`, последний параметр - путь до файла со скриптом для бота.
- При необходимости пересборки проекта можно для каждого из модулей `server`, `client`, `bot_mg` в их директориях предварительно выполнить команду: `make clean`
## Описание скриптового языка
На данный момент язык довольно прост. 
### Общее описание
Программа на этом языке состоит из операторов, каждый из которых может быть помечен меткой. Для упрощения анализа имя метки начинается всегда с символа `@` и заканчивается `:`; имя метки может состоять из латинских букв, цифр и знаковподчёркивания. Если оператор начинается не с метки, считается, что этот оператор не имеет метки. Конец оператора обозначается `;`.
В языке присутствуют следующие операторы:
- оператор присваивания
- оператор безусловного перехода(`goto`)
- условный оператор(`if`)
- операторы игровых действий(`buy`, `sell`, `prod`, `build`, `endturn`)
- оператор отладочной печати(`print`)
### Арифметика и выражения
В языке поддерживаются арифметические операции:
- сложение(+), вычитание(-)
- умножение(\*), деление(/), взятие остатка от деления(%)
- операции сравнения(<, >, =)
- логические операции(&, |, !)
- унарный минус

Операции сравнения выдают значение 1 для истины, 0 для лжи

Наибольший приоритет имеют умножение, деление и остаток, далее сложение и вычитание, операции сравнения имеют низший приоритет

В качестве операндов могут выступать `константы`, `переменные`, `обращения к встроенным функциям`

В выражениях могут присутствовать круглые скобки любой вложенности.
### Переменные и присваивания
Переменные в языке имеют имена, начинающиеся с знака `$`. В имени переменной могут
присутствовать латинские буквы, цифры, знак подчёркивания. Переменные заносятся в спец.
таблицу в момент первого присваивания.

Оператор присваивания имеет следующий синтаксис:
`<присваивание> ::= <переменная> = <выражение> ';'`
`<переменная> ::= <имя_переменной>`

По обе стороны от знака присваивания допустимо любое кол-во пробельных символов.
### Условный оператор
Условный оператор имеет следующий синтаксис:
`<условный оператор> ::= 'if' <выражение> 'then' <оператор> ';'`



## Краткое описание порядка игры

