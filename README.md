# Игра "Менеджмент"
Данная игра была предложена фирмой Avalon Hill Company для обучения основам управления предприятием. Ч.Уэзерелл отметил чрезвычайную привлекательность этой игры в качестве упражнения для
программистов. В этом репозитории находится моя реализация этой игры на чистом Си. Некоторые части программы также реализованы на C++ без использования каких-либо сторонних библиотек, контейнеров STL и т.д.
## Состав проекта
Проект сделан по клиент-серверной архитектуре. Клиентом может быть как реальный игрок, так и компьютер, стратегия поведения которого задаётся скриптовыми текстовыми файлами на специально созданном для него языке.
Все скрипты должны находиться в директории `game_scripts`. Краткое описание скриптового языка будет представлено далее.
