Los autores de este proyecto son:
Martí Vich Gispert
Joaquín Esperon Solari
Marc Nadal Saster Gondar


En este documento explicaremos algunos aspectos a tener en cuenta sobre nuestro proyecto:

Las mejoras que hemos implementado en este proyecto son la implementación de mi_touch.c y mi_rm_dir.c.

También hemos implementado mi_ls para obtener el nombre de las entradas y la posibilidad de obtener información extra con mi_ls -l. Además, hemos puesto colores para diferenciar entre ficheros y directorios. También hemos hecho que la función pueda obtener la información de los ficheros.

También hemos creado el documento mi_rm_r.c para eliminar directorios que puedan no estar vacíos, y un script "scrip_rm_r.sh" para comprobar el correcto funcionamiento de esta función recursiva.

Encontramos también la mejora que consiste en implementar caché de ficheros para las lecturas y escrituras. Eso sí, no hemos implementado las tablas FIFO ni las tablas LRU.

Además, hemos mejorado liberar_bloques_inodo mediante saltar la exploración de bloques innecesarios y minimizar la cantidad de breads y bwrites.
