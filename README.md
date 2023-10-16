# tp-2022-1c-MargeCreoQueOdioOperativos
Integrantes:
* Antonella Bevilacqua
* Matias Balboa
* Matias Oter
* Nicolás

Módulos y responsables:
* **Consola** (Matias Oter)
* **Kernel** (Antonella Bevilacqua)
* **CPU** (Matias Oter y Nicolás)
* **Memoria y Swap** (Matias Balboa)

Objetivo del Trabajo Práctico:
El objetivo del trabajo práctico consiste en desarrollar una solución que permita la simulación de un sistema distribuido, donde se tendrán que planificar procesos, resolver peticiones al sistema y administrar de manera adecuada una memoria bajo el esquema de paginación jerárquica de dos niveles.
Para el desarrollo del mismo se decidió la creación de un sistema bajo la metodología Iterativa Incremental donde se solicitarán en una primera instancia la implementación de ciertos módulos para luego poder realizar una integración total con los restantes

Módulos:
![image](https://github.com/antobevi/tp-2022-1c-SO-MargeCreoQueOdioOperativos/assets/48884370/79d5f21b-5798-4329-84ce-f3cd308c92c6)
El trabajo práctico consiste de 4 módulos: **Consola** (N instancias), **Kernel**, **CPU** y **Memoria** (1 instancia cada uno).
El proceso de ejecución del mismo consiste en crear procesos a través de instancias del módulo Consola, las cuales enviarán la información necesaria al módulo Kernel para que el mismo pueda crear las estructuras necesarias a fin de administrar y planificar su ejecución mediante diversos algoritmos. Estos procesos serán ejecutados en el módulo CPU, quien interpretará sus instrucciones y hará las peticiones a Memoria que fuera necesarias. Este último módulo implementará un esquema de memoria virtual (paginación a demanda), por lo que incorporará también un área de SWAP.
Una vez que un proceso finalice tras haber sido ejecutadas todas sus instrucciones, el Kernel devolverá un mensaje de finalización a su Consola correspondiente y cerrará la conexión.
