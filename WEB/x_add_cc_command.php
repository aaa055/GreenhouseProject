<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $list_index = intval(@$_GET['list_index']);
   $command_action = intval(@$_GET['command_action']);
   $command_param = intval(@$_GET['command_param']);
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("INSERT INTO composite_commands(controller_id,list_index,command_action,command_param) VALUES($controller_id,$list_index,$command_action,$command_param);");
      $dbengine->commitTransaction();
    }   
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>