<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $connect_to_router = intval(@$_GET['connect_to_router']);
   $router_id = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['router_id']),0,50));
   $router_pass = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['router_pass']),0,50));
   $station_id = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['station_id']),0,50));
   $station_pass = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['station_pass']),0,50));
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("DELETE FROM wifi_settings WHERE controller_id=$controller_id;");
      $dbengine->exec("INSERT INTO wifi_settings(controller_id,connect_to_router,router_id,router_pass,station_id,station_pass) VALUES($controller_id,$connect_to_router,'$router_id','$router_pass','$station_id','$station_pass');");
      $dbengine->commitTransaction();
    }
    
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>