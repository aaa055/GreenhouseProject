<?php
// подключаем базовые определения
require("base_define.php");

// подключаем общий функционал
require_once(INCLUDE_PATH . "utils/common.php");

// подключаем Smarty
require_once(INCLUDE_PATH . "utils/smarty.config.php");

// подключаем работу с базой
require_once(INCLUDE_PATH . "utils/database.config.php");

// подключаем работу с сокетами
require_once(INCLUDE_PATH . "utils/socket_transport.php");


$authorized = true; // проверяем авторизацию, если авторизован - выставляем данные сессии


 // говорим шаблонному движку - авторизован ли пользователь
$tplEngine->assign('authorized',$authorized);

// получаем данные о контроллерах 
$controllers = array();

$res = $dbengine->query("SELECT * FROM controllers;"); 
  // В цикле выведем все полученные данные 
  while ($array = $res->fetchArray())  
  { 
    $controllers[] = $array;
  } 


$selected_controller = null;
$selected_controller_id = intval(@$_GET['id']);
if($selected_controller_id > 0)
{
  foreach($controllers as $k => $v)
  {
    if($v['controller_id'] == $selected_controller_id)
    {
      $selected_controller = $v;
      break;
    }
  }
  
  if($selected_controller == null) // если запросили ID контроллера и он не найден в БД - перенаправляем на главную страницу
  {
    header('Location: /');
    exit;
  }
}


$tplEngine->assign("controllers",$controllers);
$tplEngine->assign("selected_controller", $selected_controller);



if($authorized)
{
  // добавляем в шаблонный движок другую информацию о пользователе

} // if($authorized)


?>