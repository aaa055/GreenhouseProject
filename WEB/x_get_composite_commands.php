<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем настройки составных команд
$composite_commands = array();

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
    
      // выбираем настройки из базы
      $res = $dbengine->query("SELECT cc.*, cl.list_name FROM composite_commands AS cc LEFT JOIN cc_lists AS cl ON(cl.list_index == cc.list_index) WHERE cc.controller_id=$controller_id ORDER BY cc.list_index, cc.command_action;"); 
      while($array = $res->fetchArray())
      {
       $composite_commands[] = $array;
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'composite_commands' => $composite_commands);

// отсылаем его юзеру
echo json_encode($json_data);

?>