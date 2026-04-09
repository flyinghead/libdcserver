/*
	Utility library for Dreamcast game servers.
    Copyright (C) 2026  Flyinghead

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "internal.h"
#include <stdexcept>

WorkerThread::WorkerThread()
{
	thread = std::make_unique<std::thread>([this] {
		while (true)
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(mutex);
                condition.wait(lock, [this] { return stopping || !tasks.empty(); });
                if (stopping && tasks.empty())
                	return;
                task = std::move(tasks.front());
                tasks.pop();
            }
			try {
				task();
			} catch (const std::exception& e) {
				fprintf(stderr, "WorkerThread exception: %s\n", e.what());
			} catch (...) {
				fprintf(stderr, "WorkerThread: unknown exception\n");
			}
        }
    });
}

WorkerThread::~WorkerThread()
{
    {
        std::lock_guard<std::mutex> _(mutex);
        stopping = true;
    }
    condition.notify_all();
    if (thread->joinable())
    	thread->join();
}

void WorkerThread::run(std::function<void()>&& task)
{
    {
        std::lock_guard<std::mutex> _(mutex);
        if (stopping)
        	return;
        tasks.push(std::move(task));
    }
    condition.notify_one();
}
