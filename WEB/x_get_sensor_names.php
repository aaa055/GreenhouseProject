<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем список имён датчиков
$names = array();

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
    
      // обновляем базу
      $res = $dbengine->query("SELECT * FROM sensor_names WHERE controller_id=$controller_id;"); 
      while($array = $res->fetchArray())
      {
       $names[] = $array;
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'names' => $names);

// отсылаем его юзеру
echo json_encode($json_data);

?>