<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("DELETE FROM cc_lists WHERE controller_id=$controller_id;");
      $dbengine->exec("DELETE FROM composite_commands WHERE controller_id=$controller_id;");
      $dbengine->commitTransaction();
    }   
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>