<?php
header("content-type:text/json;charset=utf-8");
$link = mysqli_connect("127.0.0.1", "root", "toor", "sensors");
if (!$link)
{
    echo "Debugging error: " . mysqli_connect_error() . PHP_EOL;
    exit;
}
//| 209 |    0 |    0 |        0 | 2018-11-09 15:34:09 |
$sql = "SELECT ts,ppb2mgm3,mgm3 FROM HOCH;";
$array=array();
$data="";
mysqli_query($link, "SET NAMES utf8");
$ret = mysqli_query( $link, $sql );
while($row = mysqli_fetch_array($ret))
{
    $array[] = ["time" => $row['ts'],"ppb2mgm3" => $row['ppb2mgm3'],"mgm3"=>$row['mgm3'],"std"=>"0.08"];
}
$data=json_encode($array);
echo $data;
mysqli_close($link);
?>
