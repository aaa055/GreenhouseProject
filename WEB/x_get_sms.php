<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем настройки составных команд
$sms_list = array();

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
    
      // выбираем настройки из базы
      $res = $dbengine->query("SELECT * FROM sms WHERE controller_id=$controller_id ORDER BY sms_text;"); 
      while($array = $res->fetchArray())
      {
       $sms_list[] = $array;
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'sms_list' => $sms_list);

// отсылаем его юзеру
echo json_encode($json_data);

?>