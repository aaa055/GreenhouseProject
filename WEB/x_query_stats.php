<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем статистику контроллера
$data = array();

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $from = @$_GET['from'];
   $to = @$_GET['to'];
  
    if($controller_id > 0 && $from != '' && $to != '')
    {
    
      // обновляем базу
      $sql = "SELECT st.sensor_type, st.type_description, m.module_name, m.description, cd.sensor_index, 
      cd.sensor_data, strftime('%s',cd.record_date) AS record_date
      FROM controller_data AS cd
      LEFT JOIN modules AS m ON(m.module_id=cd.module_id)
      LEFT JOIN sensor_types AS st ON(st.sensor_type_id=cd.sensor_type_id)
      WHERE cd.controller_id=$controller_id 
      AND cd.record_date BETWEEN datetime($from,'unixepoch') AND datetime($to,'unixepoch')
      ORDER BY cd.sensor_type_id, cd.record_date;";
      
      $res = $dbengine->query($sql); 
      while($row = $res->fetchArray())
      {
        $sensor_data = intval($row['sensor_data']);
        $sensorType = $row['sensor_type'];
        
        if($sensor_data < 0)
        {
            if($sensorType == 'LIGHT')
              continue;
            else
            if($sensor_data <= -127)
              continue;
        }
        
        
       $data[] = array('t' => $row['sensor_type']
          , 'd' => $row['type_description']
          , 'm' => $row['module_name']
          , 'md' => $row['description']
          , 'i' => $row['sensor_index']
          , 'sd' => $row['sensor_data']
          , 'rd' => intval($row['record_date'])*1000
       );
      }
      
      
    } // if($controller_id > 0
  
  
      
  } // if($posted == 1)
 
} // if($authorized)

//$data = array();

// создаём массив данных
$json_data = array('authorized' => $authorized, 'data' => $data);

// отсылаем его юзеру
echo json_encode($json_data);

?>