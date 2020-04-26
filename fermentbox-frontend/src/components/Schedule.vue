<template>
  <div>
    <v-list>
      <v-list-item>
        <v-list-item-content>
          <v-card outlined>
            <v-card-actions>
              <v-select v-model="currentSchedule" :items="schedules">
              </v-select>
              <v-btn icon @click="addSchedule">
                <v-icon>mdi-plus-circle-outline</v-icon>
              </v-btn>
              <v-btn icon @click="copySchedule">
                <v-icon>mdi-content-copy</v-icon>
              </v-btn>
              <v-btn icon @click="deleteSchedule">
                <v-icon>mdi-delete-outline</v-icon>
              </v-btn>
              <v-btn icon large>
                <v-icon>mdi-play</v-icon>
              </v-btn>
            </v-card-actions>
          </v-card>
        </v-list-item-content>
      </v-list-item>
      <v-list-item>
        <v-list-item-content>
          <v-card outlined>
            <v-card-title class="card-title">
              <v-icon color="secondary" style="margin-right: 5px">mdi-timer-outline</v-icon>
              {{currentSchedule}}
            </v-card-title>
            <v-card-text>
              <v-list>
                <v-list-item v-for="(task, i) in tasks" :key="i">
                  <v-list-item-icon>
                    <v-icon
                      v-if="task.controlTemperature && task.controlHumidity"
                    >mdi-oil-temperature</v-icon>
                    <v-icon v-else-if="task.controlTemperature">mdi-thermometer</v-icon>
                    <v-icon v-else-if="task.controlHumidity">mdi-water</v-icon>
                  </v-list-item-icon>
                  <v-list-item-content>
                    <v-list-item-title>{{getTitle(task)}}</v-list-item-title>
                    <v-list-item-subtitle>{{getSubTitle(task)}}</v-list-item-subtitle>
                    <v-progress-linear value="15" v-if="task.active"></v-progress-linear>
                  </v-list-item-content>
                  <v-list-item-action>
                    <v-icon @click="openAddTaskDialog(task)">mdi-pencil</v-icon>
                  </v-list-item-action>
                  <v-list-item-action>
                    <v-icon @click="removeTask(task)">mdi-delete</v-icon>
                  </v-list-item-action>
                </v-list-item>
              </v-list>
            </v-card-text>
          </v-card>
          <v-btn absolute dark fab bottom right color="accent" @click="openAddTaskDialog()">
            <v-icon>mdi-plus</v-icon>
          </v-btn>
        </v-list-item-content>
      </v-list-item>
    </v-list>

    <v-dialog v-model="showAddTaskDialog">
      <v-card>
        <v-card-title>
          <v-icon color="primary">mdi-calendar-check</v-icon>
          <span class="card-title">Add Task</span>
        </v-card-title>
        <v-card-text>
          <v-slider
            v-model="addTaskDialog.duration"
            class="align-center control"
            :min="0"
            :max="100"
            hide-details
            label="Duration"
            thumb-label
          >
            <template v-slot:append>
              <v-text-field
                v-model="addTaskDialog.duration"
                class="mt-0 pt-0"
                hide-details
                type="number"
                style="width: 60px"
                suffix="h"
              ></v-text-field>
            </template>
          </v-slider>
          <v-switch
            v-model="addTaskDialog.controlTemperature"
            class="ma-2"
            label="Control Temperature"
          ></v-switch>
          <v-slider
            v-model="addTaskDialog.temperature"
            class="align-center"
            :min="0"
            :max="60"
            hide-details
            label="Temperature"
            thumb-label
            :disabled="!addTaskDialog.controlTemperature"
          >
            <template v-slot:append>
              <v-text-field
                v-model="addTaskDialog.temperature"
                class="mt-0 pt-0"
                hide-details
                type="number"
                style="width: 60px"
                suffix="°C"
              ></v-text-field>
            </template>
          </v-slider>
          <v-switch v-model="addTaskDialog.controlHumidity" class="ma-2" label="Control Humidity"></v-switch>
          <v-slider
            v-model="addTaskDialog.humidity"
            class="align-center control"
            :min="0"
            :max="100"
            hide-details
            label="Humidity"
            thumb-label
            :disabled="!addTaskDialog.controlHumidity"
          >
            <template v-slot:append>
              <v-text-field
                v-model="addTaskDialog.humidity"
                class="mt-0 pt-0"
                hide-details
                type="number"
                style="width: 60px"
                suffix="%"
              ></v-text-field>
            </template>
          </v-slider>
        </v-card-text>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="accent" outlined @click="showAddTaskDialog = false">Close</v-btn>
          <v-btn color="accent" depressed @click="addTask" v-if="addTaskDialog.edit">Add Task</v-btn>
          <v-btn color="accent" depressed @click="editTask" v-else>Update Task</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </div>
</template>

<style lang="scss" scoped>
@import "../style/styles.scss";
.card-title {
  color: $color-secondary;
  margin-left: 5px;
}
</style>

<script>
export default {
  name: "Schedule",

  data: () => ({
    tasks: [], // TODO offer option to reorder tasks
    schedules: ["Schedule 1", "Schedule 2", "Schedule 3"],
    currentSchedule: "Schedule 1",
    showAddTaskDialog: false,
    addTaskDialog: {
      temperature: 0,
      humidity: 0,
      duration: 1,
      controlTemperature: true,
      controlHumidity: true,
      edit: false
    }
  }),

  computed: {
    dialogType: function() {
      return this.addTaskDialog.label.toLowerCase();
    }
  },

  created: function() {
    this.loadSchedules();
    this.loadTasks();
  },

  methods: {
    loadSchedules() {
      // TODO load schedules
    },
    addSchedule() {
      // TODO add schedule
      alert("add schedule");
    },
    copySchedule() {
      // TODO copy schedule
      alert("copy schedule");
    },
    deleteSchedule() {
      // TODO delete schedule
      alert("delete schedule");
    },
    loadTasks: function() {
      // TODO load tasks
      this.tasks = [
        {
          id: "temp123",
          temperature: 30,
          humidity: 90,
          duration: 1,
          controlTemperature: true,
          controlHumidity: true,
          active: true
        },
        {
          id: "humi123",
          temperature: 60,
          humidity: 20,
          duration: 4,
          controlTemperature: true,
          controlHumidity: false,
          active: false
        },
        {
          id: "humi456",
          temperature: 60,
          humidity: 20,
          duration: 4,
          controlTemperature: false,
          controlHumidity: true,
          active: false
        }
      ];
    },
    storeTasks: function() {
      // TOO store tasks
    },
    getTitle: function(task) {
      let title = "";
      if (task.controlTemperature) {
        title = `${title}${task.temperature}°C Temperature`;
      }
      if (task.controlTemperature && task.controlHumidity) {
        title = `${title}, `;
      }
      if (task.controlHumidity) {
        title = `${title}${task.humidity}% Humidity`;
      }
      return title;
    },
    getSubTitle: function(task) {
      return `Duration: ${task.duration} hours`;
    },
    openAddTaskDialog: function(task) {
      this.addTaskDialog.task = task;
      this.addTaskDialog.controlTemperature = task.controlTemperature;
      this.addTaskDialog.controlHumidity = task.controlHumidity;
      this.addTaskDialog.temperature = task.temperature;
      this.addTaskDialog.humidity = task.humidity;
      this.addTaskDialog.duration = task.duration;

      this.showAddTaskDialog = true;
    },
    addTask: function() {
      // TODO validate input
      this.tasks.push({
        id: this.generateId(),
        controlTemperature: this.addTaskDialog.controlTemperature,
        controlHumidity: this.addTaskDialog.controlHumidity,
        temperature: this.addTaskDialog.temperature,
        humidity: this.addTaskDialog.humidity,
        duration: this.addTaskDialog.duration
      });
      this.showAddTaskDialog = false;
    },
    editTask: function() {
      // TODO validate input
      const task = this.addTaskDialog.task;
      task.controlTemperature = this.addTaskDialog.controlTemperature;
      task.controlHumidity = this.addTaskDialog.controlHumidity;
      task.temperature = this.addTaskDialog.temperature;
      task.humidity = this.addTaskDialog.humidity;
      task.duration = this.addTaskDialog.duration;
      this.showAddTaskDialog = false;
    },
    removeTask: function(task) {
      this.tasks = this.tasks.filter(t => t.id !== task.id);
    },
    generateId() {
      return `${Date.now()}`;
    }
  }
};
</script>
