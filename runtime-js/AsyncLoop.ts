import { timeout } from "./jsUtils";

export type QueueF = (task: Task) => void;
export type Task = (queue: QueueF) => Promise<void>;

export class AsyncLoop {
  constructor(public loopTasks: Task[] = []) {
    this._startLoop();
  }
  queue(task: Task) {
    this.loopTasks.push(task);
  }
  clear() {
    // NB: Very important this sets to an entirely new list!
    //
    // This means we can robustly clear any tasks - they will
    // queue children tasks, but effectively be 'no-ops'
    // as they will be pushed to a dangling list (i.e, no longer used in our loop).
    this.loopTasks = [];
  }
  private async _startLoop() {
    // For now, we keep loops going indefinitely...
    while (true) {
      // NB: Very important we look up and capture the list
      // at this instance. This enables clear() to set up no-ops.
      const loopTasks = this.loopTasks;
      // First-in-first out consideration of tasks
      const task = loopTasks.shift();
      if (task) {
        await task((task) => loopTasks.push(task));
      } else {
        // If no task is given, wait 15ms before polling again
        // TODO we could end the loop and awaken on queue(),
        // but for now this is robust.
        await timeout(15);
      }
    }
  }
}
