import React from "react";
import { Line } from "react-chartjs-2";
import { useEffect, useState } from "react";
import Chart from "chart.js/auto";

const Use_swap = ({ip,port}) => {

    const [ejeX, setEjeX] = useState(0);

    const [data, setData] = useState({
        labels: [],
        datasets: [
            {
                label: "Used Physical Memory (MB)",
                data: [],
                fill: false,
                borderColor: 'rgb(75, 192, 192)',
                tension: 0.1,
            },
            {
                label: "Used Swap Memory (MB)",
                data: [],
                fill: false,
                borderColor: 'rgb(192, 75, 75)',
                tension: 0.1,
            },
        ],
    });

    function update_swap() {
        const url = `http://${ip}:${port}/use_swap`;
        fetch(url)
          .then((res) => res.json())
          .then((use_swap) => {
            setData((prevData) => ({
                labels: [...prevData.labels, prevData.labels.length],
                datasets: [
                    {
                        ...prevData.datasets[0],
                        data: [...prevData.datasets[0].data, use_swap.used_fisic],
                    },
                    {
                        ...prevData.datasets[1],
                        data: [...prevData.datasets[1].data, use_swap.used_swap],
                    },
                ],
            }));
            setEjeX((prevEjeX) => prevEjeX + 1);
          })
          .catch((err) => console.log(err));
    }

    useEffect(() => {
        const interval = setInterval(() => {
            update_swap();
        }, 1000);
        return () => clearInterval(interval);
    }, []);

    return (
        <>
            <Line data={data} />
        </>
    );
};

export default Use_swap;
