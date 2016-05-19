<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $new_id = intval(@$_GET['new_id']);
   $controller_name = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['controller_name']),0,50));
   $c_addr = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['c_addr']),0,100));
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("UPDATE controllers SET controller_name='$controller_name', controller_address='$c_addr' WHERE controller_id=$controller_id ;");
      
      if($new_id > 0 && $new_id != $controller_id) // обновить ID контроллера
      {
        $dbengine->exec("UPDATE controllers SET controller_id=$new_id WHERE controller_id=$controller_id ;");
        // тут остальные обновления!!!
      }
      $dbengine->commitTransaction();
    }
    
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>