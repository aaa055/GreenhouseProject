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
   $list_name = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['list_name']),0,100));
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("INSERT INTO cc_lists(controller_id,list_index,list_name) VALUES($controller_id,$list_index,'$list_name');");
      $dbengine->commitTransaction();
    }   
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>