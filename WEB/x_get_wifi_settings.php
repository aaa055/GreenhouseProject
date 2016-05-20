<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем настройки wi-fi
$wifi = array();

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
    
      // выбираем настройки из базы
      $res = $dbengine->query("SELECT * FROM wifi_settings WHERE controller_id=$controller_id;"); 
      if($array = $res->fetchArray())
      {
       $wifi = $array;
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'wifi' => $wifi);

// отсылаем его юзеру
echo json_encode($json_data);

?>