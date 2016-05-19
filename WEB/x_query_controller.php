<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// получаем данные с контроллера
$query_result = "ER=OFFLINE";

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
   $query = @$_GET['query'];
  
    if($controller_id > 0)
    {
    
      $res = $dbengine->query("SELECT * FROM controllers WHERE controller_id=$controller_id;"); 
      if($array = $res->fetchArray())
      {
        $online = $array['is_online'];
        if($online)
        {
             $tp = new SocketTransport();
            if($tp->open($array['controller_address']))
            {
              $query_result = $tp->ctget($query);
              $tp->close();
            }
         
        }
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'query_result' => $query_result);

// отсылаем его юзеру
echo json_encode($json_data);

?>