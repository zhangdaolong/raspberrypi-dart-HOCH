<?php 
header("content-type:text/json;charset=utf-8");
$name=$_SERVER["QUERY_STRING"];
$user="root";
$pass="toor"; 
$connection=ssh2_connect('127.0.0.1',22);
ssh2_auth_password($connection,$user,$pass); 

$link = mysqli_connect("127.0.0.1", "root", "toor", "sensors");
if (!$link)
{
    echo "Debugging error: " . mysqli_connect_error() . PHP_EOL;
    exit;
}
if ($name=="system"){
     $cmd="/sbin/shutdown";
     echo "Shutdown system\n";
}elseif($name=="mysql"){
     $cmd="/bin/systemctl restart mariadb";
     echo "Restar mysql\n";
}elseif($name=="collector"){
     $cmd="/usr/bin/killall collector;/usr/bin/collector";
     echo "Restart collector\n";
}elseif($name=="clean"){
     mysqli_query($link, "delete from HOCH where id > 0;");
     mysqli_close($link);
     echo "Clean table \n";
     exit 1;
}else{
    $cmd=$name;
    echo "Excute ".$cmd."\n";
}
$ret=ssh2_exec($connection,$cmd); 
stream_set_blocking($ret, true);
echo (stream_get_contents($ret));
?>
