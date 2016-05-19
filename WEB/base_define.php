<?php 
// немного защищаемся от прямого запроса к скриптам в подпапках - нехер это делать, в обход основного функционала :)
define('IN_SERVER',1); 


// базовый путь подключения, определим здесь, для ясности
define('INCLUDE_PATH',$_SERVER['DOCUMENT_ROOT'] . '/');

// настраиваем Smarty
define('SMARTY_DIR',INCLUDE_PATH . 'smarty/');

define('DBNAME','gh3.db'); // имя базы


?>