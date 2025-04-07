import { makeStyles } from "@fluentui/react-components";

const authUseStyles = makeStyles({
  // Styles for the main container
  loginContainer: {
    display: "flex",
    flexDirection: "column",
    justifyContent: "flex-start", // Changed from center to flex-start
    alignItems: "center",
    padding: "20px",
    boxSizing: "border-box",
    marginTop: "0px", // Ensure no additional margin
    minHeight: "100vh",
  },
  // Styles for the Stack that acts as a card
  stack: {
    maxWidth: "500px",
    width: "100%",
    padding: "40px",
    backgroundColor: "white",
    borderRadius: "8px",
    boxShadow: "0 2px 10px rgba(0, 0, 0, 0.1)",
  },
  // Styles for the TextField component
  textfield: {
    width: "100%",
    marginBottom: "6px",
    // Nested selectors for the TextField's internal elements
    selectors: {
      "& .ms-TextField-fieldGroup": {
        height: "44px !important",
      },
      "& label": {
        fontWeight: 600,
        marginBottom: "8px",
      },
      "&:focus-within": {
        borderColor: "#0078d4",
        boxShadow: "0 0 0 2px rgba(0, 120, 212, 0.2)",
      },
    },
  },
  // Styles for buttons (applied to components with a custom class)
  button: {
    width: "100%",
    marginTop: "80px",
  },
  // Override for the DatePicker arrow if needed
  datePickerArrow: {
    color: "black",
    backgroundColor: "black",
  },
  iconCheckmark: {
    color: "green",
    margin: "8px",
  },
  iconError: {
    color: "red",
    margin: "8px",
  },
  datePicker: {
    maxWidth: "150px",
    maxHeight: "35px",
    marginBottom: "15px",
  },
  alreadyRegistered: {
    display: "flex",
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center",
  },
  link: {
    marginLeft: "6px",
  },
});

export default authUseStyles;
