<?php
// эта строчка обязательна к подключению всеми скриптами
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

// проверяем, онлайн ли выбранный контроллер
$online = 0;

if($authorized)
{
  $posted = intval(@$_GET['posted']);
  if($posted == 1)
  {
   $controller_id = intval(@$_GET['controller_id']);
  
    if($controller_id > 0)
    {
    
      // обновляем базу
      $res = $dbengine->query("SELECT * FROM controllers WHERE controller_id=$controller_id;"); 
      if($array = $res->fetchArray())
      {
        $online = $array['is_online'];
        $tp = new SocketTransport();
        if($tp->open($array['controller_address']))
        {
          $online = 1;
          $tp->close();
        }
        else
        {
          $online = 0;
        }
        if($online != $array['is_online'])
        {
          $dbengine->beginTransaction();
          $dbengine->exec("UPDATE controllers SET is_online=$online WHERE controller_id=$controller_id;");
          $dbengine->commitTransaction();
        }
      }
      
      
    }
  
  
      
  } // if
 
}


// создаём массив данных
$json_data = array('authorized' => $authorized, 'online' => $online);

// отсылаем его юзеру
echo json_encode($json_data);

?>