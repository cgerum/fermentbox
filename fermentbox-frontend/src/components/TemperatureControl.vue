<template>
  <v-card outlined>
    <v-card-title>
      <v-icon x-large color="secondary">mdi-thermometer</v-icon>
      <span class="card-title">Temperature</span>
    </v-card-title>
    <v-card-text>
    <v-sparkline
      :gradient="['#f72047', '#ffd200', '#1c449a']"
      :line-width="3"
      :smooth="16"
      :value="temperatureLog"
      :labels="temperatureLogLabels"
      auto-draw
      stroke-linecap="round"
    ></v-sparkline>
    <v-slider
      v-model="temperature"
      class="align-center"
      :min="0"
      :max="60"
      hide-details
      thumb-label
    >
      <template v-slot:append>
        <v-text-field
          v-model="temperature"
          class="mt-0 pt-0"
          hide-details
          type="number"
          style="width: 60px"
          suffix="°C"
        ></v-text-field>
      </template>
    </v-slider>
    </v-card-text>
  </v-card>
</template>

<style lang="scss" scoped>
@import "../style/styles.scss";
.card-title {
  color: $color-secondary;
}
</style>

<script>
export default {
  name: "TemperatureControl",

  data: () => ({
    temperature: 20,
    temperatureLog: [
      { label: "10.4.", value: 5 },
      { label: "11.4.", value: 5 },
      { label: "12.4.", value: 5 },
      { label: "13.4.", value: 5 },
      { label: "14.4.", value: 5 },
      { label: "15.4.", value: 20 },
      { label: "16.4.", value: 5 },
      { label: "17.4.", value: 50 }
    ]
  }),
  computed: {
    temperatureLogLabels: function() {
      return this.temperatureLog.map(data => data.label);
    }
  },
  methods: {
    updateTemperatureLog: function() {
      // TODO update temperature log
    }
  }
};
</script>
