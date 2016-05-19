<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $sensor_idx = intval(@$_GET['sensor_idx']);
   $module_name = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['module_name']),0,50));
   $display_name = $dbengine->escapeString(mb_substr(strip_tags(@$_GET['display_name']),0,100));
  
    if($controller_id > 0)
    {
      // обновляем базу
      $dbengine->beginTransaction();
      $dbengine->exec("DELETE FROM sensor_names WHERE controller_id=$controller_id AND sensor_idx=$sensor_idx AND module_name='$module_name';");
      $dbengine->exec("INSERT INTO sensor_names(controller_id,sensor_idx,module_name,display_name) VALUES($controller_id,$sensor_idx,'$module_name','$display_name');");
      $dbengine->commitTransaction();
    }
    
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized);

// отсылаем его юзеру
echo json_encode($json_data);

?>