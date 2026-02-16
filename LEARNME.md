escribo esto a las 12:26 del 14 de febredo del 2026, voy a intentar reconstruir los hechos de ayer

Hecho: bug, el sistema no encontraba cosas que agregue
Hice:  deje de usar el pool de la STL, actualize a una version meor de gcc porque pense que era un bug, hice de todo
Aprendi: que en un pool debes dejar memoria vacia al inicio porque si usas pointer == 0 para saber si es nullptr resulta que el primer puntero es laa direccion 0


______

Hecho: mi metodo tardaba hasta 300 ns mientras la stl tenia un promeio de 150 ns con el mejor tiempo de 70 ns por parte del vector
Hipotesis: mi funcion FlashSearch::Search es demasiado segura, verifica todo para evitar errores como SEGFAULT
Aprendi: La seguridad es buena pero costosa
Lo solucione: cree FlashSearch::fast_search, una funcion inline rapida y compacta, si te equivocas da un segfault pero dio un tiempo de 25 ns en una longitud de 2


______

Hecho: no estaba probando "bien", el dataset era de apenas 2 palabras
Solucion: descargue un diccionario y cree un analizador para usar e diccionario entero y use random para elegir una paabra al azar
Aprendi: la prueba real no es con 2 palabras sino con un diccionario entero


______

Hecho: El vector tarda milenios con el diccionario dependiendo de con ue letra inicie la palabra, pueden ver a detalle en Benchmarks.txt, y usaba al CPU al 100%
Solucion: Agregue la opcion de incluirlo o no.


______

Hecho: Mi FlashSerach::fast_search es extremadamente dependiente de la lonitud, con palabras largas unordered set y unordered map le ganaban
Solucion: use prefetch para que la cpu vaya precargando los siguientes bloques en la L1 y en vez de solo iterar 100000 busquedas de a misma palabra agregue tests de diferentes palabras al azar con 100000 busquedas en cada una y 10 tests.

** Ahora pasare a narrar los hechos de hoy **

______

Hecho: El problema es que el prefectch ayudo en palabras largas pero en cortas el costo de procesar el prfecth alargo el tiempo
Solucion: Le agregue un template a fast_search para elegir WithPrefetch, use un template paar hacer el if una sola vez y no en cada ciclo.
Correcion:resulta que los templates son en tiempo de compilacion, lo reemplace por un bool WithPrefetch y 2 versiones de bucle.
Nota: recomiendo usar WithPrefetch solo si el tamaño es mayor que 11.

______

Hecho: En FlashSearch::allocate agregue una excepcion para cuando los RelativePtr se quedaban sin espacio en los 16 bits que tienen y al ejecutar casi inmediatamente dio la excepcion, esto me esta obligando a rehacer las pruebas y benchmarks por que los resutados pudieron ser erroneos
Aprendi: Antes de confiar debo verificar que no haya un bug escondido y que el hecho que que no imprima si lo encontro  no pudo haber potencialmente ocultado e bug
Solucion: En desarollo
hora: 2:10 del 2 de febrero de 2026
Actualizacion de las 2:13: Estuve pensando en cambiar dinamicamente de uint8_t a uint32_t es tecnicamente posible pero implicaria pausas en medio del agregado en donde a cpu trabaja al 100% recorriendo todos los elementos y seria muy dificil de implementar incluso para mi.
La solucion mas optima es usar un template de bool Use32Bits y en un futuro cercano Use64Bits usando probabemente enums, procedere a hacer eso y rejecutar los benchmarks
Actualisacion de las 4:01: ya aregle el bug, durante el proceso sufri inconvenientes adaptando mi logica
Nota: Esto explica los cambios repentinos no logicos del tiempo que atrivui al cache miss o que procesos de fondo lanzaban lanzaban tareas de alto costo, ahora los tiempos son mas constantes

______

Estoy creando un repositorio de github
Lo hice, esta disponibe en https://github.com/hacker-programer/Flash-Search

______

Estoy buscando como optimizar mi rendimiento, para hacerlo se me ocurre usar prefetch no solo para cargar las siguientes letras sino para cargar los siguientes bytes en la memoria para tener las siguientes letras precargadas, para hacerlo bien deberia analizar el numero promedio de hijos de un nodo

mi codigo dio
PS C:\Users\Administrator\Desktop\updown\Flash Search\build> ./FlashSearch.exe
Promedio de hijos por nodo: 1.38098
PS C:\Users\Administrator\Desktop\updown\Flash Search\build>
pero yo creo que esta sesgado, lo mas probable es que los nodos iniclaes tengan una alta densidad de hijos y los finales baja, combatire esto mostrando la estadistica por nivel.
Ahora mismo para optimisar esto bien estoy cambiando a usar punteros uint16_t siempre y diferentes paginas de 64 Kib lo que permite que cada nodo quepa en una linea de cache en vez de 2

Ya eh implementado esto, tuve un inconveniente en el proceso por no actualizar bien ReativePtr::Resolve

______

voy a impementar optimisaciones basadas en los analisis

______

pues son a 1 de a mañana, estuve toda la tarde corrigiendo bugs de las paginas, yey supongo, en este proceso me vi obligado a implementar un unit test basico, analizando los benchmarks me di cuenta que los tiempos subieron 60 ns de la version basica,  voy a analizar el porque de eso, estoy descargando google benchmark y traizy profiler de https://github.com/wolfpld/tracy/releases/tag/v0.13.1