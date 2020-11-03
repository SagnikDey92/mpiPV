read -p 'Number of processes: ' num_procs
report_dir="/users/misc/sagnikd/mpiPOutputs/"
csv_freq=5
#manually determine a cube-like 3D decomposition
nx=30
ny=30
nz=30
if [ $num_procs == 2 ]
then
   npx=1
   npy=1
   npz=2
elif [ $num_procs == 4 ]
then
   npx=1
   npy=2
   npz=2 
elif [ $num_procs == 8 ]
then
   npx=2
   npy=2
   npz=2
elif [ $num_procs == 16 ]
then
   npx=2
   npy=2
   npz=4 
elif [ $num_procs == 32 ]
then
   npx=2
   npy=4
   npz=4 
elif [ $num_procs == 64 ]
then
   npx=4
   npy=4
   npz=4
elif [ $num_procs == 128 ]
then
   npx=4
   npy=4
   npz=8 
elif [ $num_procs == 256 ]
then
   npx=4
   npy=8
   npz=8 
elif [ $num_procs == 512 ]
then
   npx=8
   npy=8
   npz=8
fi

mpiexec -f /users/misc/sagnikd/hostfile -n $num_procs /users/misc/sagnikd/Code/miniAMR/ref/miniAMR.x --npx $npx --npy $npy --npz $npz --nx $nx --ny $ny --nz $nz &

sleep 2

echo "Press [CTRL+C] to stop.."
while true
do
   python /users/misc/sagnikd/Libraries/mpiPViz/rundir/report2csv.py $report_dir
	sleep $csv_freq
done

