import React from "react";
import { Pie } from "react-chartjs-2";
import { useEffect, useState } from "react";
import Chart from "chart.js/auto";

const Use_mem = ({ip,port}) => {
    const [data, setData] = useState({
        labels: ["Used Memory", "Free Memory", "Cached Memory"],
        datasets: [
        {
        label: "Usage MB",
        backgroundColor: [
            'rgb(255, 99, 132)',
            'rgb(54, 162, 235)',
            'rgb(255, 205, 86)'
        ],
        data: [15, 10, 5],
        },
        ],
    });

    function update_ram() {
        const url = `http://${ip}:${port}/use_mem`;
        fetch(url)
          .then((res) => {
            return res.json();
          })
          .then((use_mem) => {
            setData({
                labels: ["Used Memory", "Free Memory", "Cached Memory"],
                datasets: [
                {
                label: "Usage MB",
                backgroundColor: [
                    'rgb(255, 99, 132)',
                    'rgb(54, 162, 235)',
                    'rgb(255, 205, 86)'
                ],
                data: [use_mem.used, use_mem.free, use_mem.cached],
                },
                ],
            });
          })
          .catch((err) => console.log(err));
    }   
    
    useEffect(() => {
        const interval = setInterval(() => {
            update_ram();
        }, 1000);
        return () => clearInterval(interval);
    }, []);

    return (
    <div>
        <Pie data={data} />
    </div>
    );
};
export default Use_mem;